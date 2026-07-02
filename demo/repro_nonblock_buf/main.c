/*
 * Minimal headless reproducer for the nk_nonblock_begin popup-buf bug.
 *
 * Root cause
 * ----------
 * When a renderer caches the previous frame's output and skips nk_foreach
 * on frames where nothing changed (e.g. a hash/dirty check), nk_build is
 * never called for those frames.  nk_build is the only place that links
 * the popup command buffer into the main chain — and, crucially, it is the
 * only code path that can detect that a stale buf->active must be ignored.
 *
 * When the popup was open on such a "skipped" frame:
 *   - nk_start_popup set buf->active = nk_true and recorded buf->begin /
 *     buf->last / buf->parent as offsets into the current frame's buffer.
 *   - nk_clear reset the buffer without nk_build having run, so buf->active
 *     stays nk_true and the recorded offsets are now stale.
 *
 * On the next frame when the popup closes:
 *   - nk_nonblock_begin returns nk_false via the !is_active early-return
 *     path without calling nk_start_popup (so buf->active is not refreshed)
 *     and WITHOUT clearing buf->active (the missing line the fix adds).
 *   - nk_end -> nk_finish sees buf->active == nk_true and links stale
 *     offsets into the fresh frame's command chain.
 *   - nk_foreach -> nk_build follows the stale chain and emits draw calls
 *     from the previous frame's popup, even though the popup is now closed.
 *
 * Observed failure
 * ----------------
 * The command count seen by nk_foreach on the close-frame is wrong: it
 * includes all popup-body draw calls from the frame before the skip, even
 * though the popup has already closed.  This demo detects this by comparing
 * the command count against a known-clean baseline (frame 0, no popup open).
 *
 * Build
 * -----
 *   gcc -O1 -I../.. main.c -o repro && ./repro
 *   (optionally add -fsanitize=undefined)
 *
 * Expected output without fix : "FAIL" with non-zero exit code
 * Expected output with fix    : "ok" printed, exit 0
 *
 * Frame sequence
 * --------------
 * Sending press+release in the same input frame leaves btn->down=0 so
 * nk_input_is_mouse_pressed (= down && clicked) is false and the combo
 * header button never registers as clicked.  Separating press and release
 * across frames ensures that when the combo opens btn->down=1 and
 * btn->clicked=1 satisfy the pressed check together.
 *
 *   Frame 0 – no input        : baseline (popup closed)    count_baseline
 *   Frame 1 – press at combo  : combo opens                (hash changes)
 *   Frame 2 – release         : popup stays open           (hash changes) nk_foreach runs
 *   Frame 3 – no input        : popup stays open           (hash matches) nk_foreach SKIPPED → BUG
 *   Frame 4 – press elsewhere : popup should close         count must equal count_baseline
 */

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_IMPLEMENTATION
#include "../../nuklear.h"

/* Minimal stub font — no actual glyph rendering needed. */
static float stub_width(nk_handle h, float height, const char *text, int len)
{ (void)h; (void)height; (void)text; return (float)len * 8.0f; }

static void do_layout(struct nk_context *ctx, int *selected)
{
    static const char *items[] = { "alpha", "beta", "gamma" };
    if (nk_begin(ctx, "W", nk_rect(0, 0, 400, 400), 0)) {
        nk_layout_row_static(ctx, 25, 200, 1);
        *selected = nk_combo(ctx, items, 3, *selected, 25, nk_vec2(200, 150));
    }
    nk_end(ctx);
}

/*
 * Count the commands that nk_foreach would emit this frame and return it.
 * On a hash miss (normal path) nk_foreach runs as usual.
 * On a hash hit  (lazy path)  nk_foreach is skipped — this is the bug site.
 */
static nk_uint s_last_hash = 0;

static int render_or_skip(struct nk_context *ctx, int frame)
{
    const struct nk_command *cmd;
    int count = 0;

    /* FNV-1a over the raw command bytes */
    nk_uint hash = 2166136261u;
    const nk_byte *p = (const nk_byte *)ctx->memory.memory.ptr;
    nk_size i;
    for (i = 0; i < ctx->memory.allocated; i++) {
        hash ^= p[i];
        hash *= 16777619u;
    }

    if (hash != s_last_hash) {
        s_last_hash = hash;
        nk_foreach(cmd, ctx) { count++; }
        fprintf(stderr, "frame %d: nk_foreach ran    buf.active=%d  cmds=%d\n",
                frame, ctx->begin ? ctx->begin->popup.buf.active : -1, count);
    } else {
        /*
         * BUG: nk_build is skipped.  nk_start_popup has already set
         * buf->active = nk_true and stored buf offsets for this frame.
         * nk_clear will reset the buffer but leave buf->active stale.
         */
        fprintf(stderr, "frame %d: hash match SKIP   buf.active=%d  (nk_foreach skipped)\n",
                frame, ctx->begin ? ctx->begin->popup.buf.active : -1);
    }
    nk_clear(ctx);
    return count;
}

int main(void)
{
    struct nk_context ctx;
    struct nk_user_font font = {0};
    int selected = 0;
    int count_baseline, count_close;

    font.height = 13.0f;
    font.width  = stub_width;
    nk_init_default(&ctx, &font);

    /* ------------------------------------------------------------------
     * Frame 0: no input — combo closed.  Record the command count as the
     * baseline.  The close-frame (frame 4) must produce the same count.
     * ------------------------------------------------------------------ */
    nk_input_begin(&ctx);
    nk_input_end(&ctx);
    do_layout(&ctx, &selected);
    count_baseline = render_or_skip(&ctx, 0);

    /* ------------------------------------------------------------------
     * Frame 1: press the combo header (do NOT release yet).
     *
     * btn->down=1 and btn->clicked=1 → nk_input_is_mouse_pressed returns
     * true.  nk_combo_begin_text calls nk_nonblock_begin which calls
     * nk_start_popup → buf->active = nk_true.  The dropdown is rendered;
     * hash changes from baseline so nk_foreach runs.
     * ------------------------------------------------------------------ */
    nk_input_begin(&ctx);
    nk_input_motion(&ctx, 100, 15);
    nk_input_button(&ctx, NK_BUTTON_LEFT, 100, 15, nk_true);
    nk_input_end(&ctx);
    do_layout(&ctx, &selected);
    render_or_skip(&ctx, 1);

    /* ------------------------------------------------------------------
     * Frame 2: release at the same position.
     *
     * After nk_input_begin resets btn->clicked to 0, the release makes
     * btn->down=0, btn->clicked=1.  nk_input_is_mouse_pressed returns
     * (0 && 1)=false → nk_nonblock_begin does NOT see a press → popup
     * stays open.  The header is now in HOVER state (not ACTIVED), so the
     * hash changes and nk_foreach runs, establishing the new baseline hash.
     * ------------------------------------------------------------------ */
    nk_input_begin(&ctx);
    nk_input_motion(&ctx, 100, 15);
    nk_input_button(&ctx, NK_BUTTON_LEFT, 100, 15, nk_false);
    nk_input_end(&ctx);
    do_layout(&ctx, &selected);
    render_or_skip(&ctx, 2);

    /* ------------------------------------------------------------------
     * Frame 3: no input — popup stays open, rendering identical to frame 2.
     *
     * Hash matches → nk_foreach is SKIPPED.
     * do_layout still calls nk_start_popup → buf->active = nk_true with
     * fresh (but soon-to-be-stale) buffer offsets.
     * nk_clear resets the buffer → offsets are NOW STALE while buf->active
     * remains nk_true.
     * ------------------------------------------------------------------ */
    nk_input_begin(&ctx);
    nk_input_end(&ctx);
    do_layout(&ctx, &selected);
    render_or_skip(&ctx, 3);

    /* ------------------------------------------------------------------
     * Frame 4: click outside the popup to close it.
     *
     * nk_nonblock_begin detects (pressed && !in_body) → is_active=false.
     *
     *   WITHOUT fix: returns without clearing buf->active.  nk_finish then
     *   links stale buf->parent/begin into the fresh command chain, and
     *   nk_foreach emits draw calls from frame 3's popup body even though
     *   the popup is now closed.  The command count is higher than baseline.
     *
     *   WITH fix: buf->active is cleared to 0 before returning.  nk_finish
     *   skips the popup chain.  The command count equals the baseline.
     * ------------------------------------------------------------------ */
    nk_input_begin(&ctx);
    nk_input_motion(&ctx, 390, 390);
    nk_input_button(&ctx, NK_BUTTON_LEFT, 390, 390, nk_true);
    nk_input_end(&ctx);
    do_layout(&ctx, &selected);
    count_close = render_or_skip(&ctx, 4);

    nk_free(&ctx);

    fprintf(stderr, "baseline cmds=%d  close-frame cmds=%d\n",
            count_baseline, count_close);

    if (count_close != count_baseline) {
        fprintf(stderr,
            "FAIL: close-frame emitted %d extra command(s) from stale popup buf\n"
            "      (nk_nonblock_begin must clear win->popup.buf.active on !is_active)\n",
            count_close - count_baseline);
        return 1;
    }

    fprintf(stdout, "ok\n");
    return 0;
}
