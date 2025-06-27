/** 
 * \file nuklear_crc.c 
 * \brief Implements CRC checking of the draw command buffer.
 * 
 * \details Implements a rolling CRC that gets updated with every draw_push
 * function call. That is, for every `nk_xxx` api function call between
 * `nk_begin()` and `nk_end()` there is one to many draw commands pushed into
 * the draw buffer.  For each one of those pushes, the command type and data is
 * hashed, then stored into that buffer's CRC member. Then on the next `nk_xxx`
 * function call, another command push is executed and the previous crc is
 * updated with the new hash of the new command. This will result in a unique CRC
 * value in the buffers crc member at the end of the draw execution. 
 *
 * The purpose of this is to allow super cheap and fast detection of unique'ness
 * between draw loops, such that the user can determine if they need to redraw
 * the screen or not. The original method was to have the user store a copy of
 * the entire draw buffer, then loop through the buffer and compare that with
 * the most recent draw buffer. Once done, you can determine if something
 * changed on the screen and needs to be `nk_convert`'d. Doing it that way would
 * require 1 loop to fill the command buffer, 1 loop to check the command buffer
 * against the previously stored buffer, then 1 more loop to copy the new buffer
 * into the old for the next draw loop iteration. That is 3 loops of
 * `command_buffer'length()` which could end up being quite large; making this
 * an expensive operation. Which almost defeats the purpose of detecting
 * unique'ness in the first place.
 * 
 * Yet implementing this CRC, we crunch all of that down into just the initial
 * draw loop. Then you have a unique code stored in a 32-bit integer that
 * represents the buffer. So if you need to know if the current draw command
 * buffer is different from the previous one; you simply need to retain a single
 * interger value and then check whether the old interger is equal to the new
 * one. We reduced 3x O(n) loop complexity and 2x O(n) memory/space complexity
 * down to 1x O(n) loop complexity and O(1) memory complexity. The CRC
 * calculation (depending on default or custom implementation) simply adds 1
 * table lookup and 1 XOR operation to a single integer. Practically a free
 * upgrade by comparison.
 *
 * If you want to use the CRC, then you must either `#define NK_DRAW_CRC`
 * or `#define NK_DRAW_CRC_CUSTOM` 
 *
 * `#define NK_DRAW_CRC` will implement the default CRC algorithm which is the
 * same murmur hash function used in the nuklear window titles.
 *
 * `#define NK_DRAW_CRC_CUSTOM` will allow the user to define their own CRC
 * implementation. This is available because most embedded systems already have
 * a CRC calculation of some kind, usually defined by their system constraints,
 * so it would be redundant to create yet another CRC method. Or, perhaps the
 * murmur hash is more expensive than what your application requires, so you can
 * implement a super simple/cheap version of your own for your needs.
 *
 * To use the `NK_DRAW_CRC_CUSTOM` the user will need to `#define
 * NK_CRC_SEED` to their desired seed and also `#define NK_CRC_FUNC(k,l,s)` to
 * the user defined implementation of the crc.
 *
 * The user defined implementation of the CRC **SHALL** have the declaration of
 * ```c
 * nk_hash <xxCustomNamexx>(const void * key, int len, nk_hash seed)
 * ```
 *
 * other than that, the user is free to use any CRC algorithm, table,
 * implementation they choose.
 *
 * \internal
 * Information for nuklear developers. Each context includes a command buffer
 * for drawing, usually called `ctx->current->buffer`. This is the buffer with
 * the draw commands and we grab the CRC value stored with this buffer
 * `ctx->current->buffer.crc`. The buffer, in regards to a context, only exists
 * to draw a panel and will _dissappear_ once `nk_end()` is called. This is
 * because nk_end commits the draw buffer to the context's overal draw array and
 * closes out the `current` buffer to wait for another `nk_begin()` to start a
 * new panel using the `nk_buffer_start()` command. 
 *
 * What this means for the CRC is that, when a user decides they want to check
 * on the CRC, they will do so **after** the `nk_end()` call (because you need
 * to make sure all of the information is there). But attempting to grab the CRC
 * at `ctx->current->buffer.crc` will result in a Seg Fault because that memory
 * was deallocated with the `nk_end()` call. But not only that, we determined
 * that the `current->buffer` is meant to be used for all sequences of
 * `nk_begin()` to `nk_end()`. Meaning that we will get new CRCs for each panel;
 * but only be able to look at the final CRC (unless we grab them as we go, but
 * thats silly when we just want to know whether to draw or not). That means we
 * better hope the user only ever interacts with which ever panel was the final
 * one drawn, otherwise we wont see a crc change.
 *
 * The solution was to store the CRC at the Context level. So now `struct
 * nk_context` contains a `crc` member that gets rolled when the current buffer
 * finishes (i.e. `nk_end()` was called). This means that, whenever
 * `nk_finish` is called, we calc the context CRC based off the current buffer
 * crc. This retains the CRC value through successive calls to `nk_begin()` and
 * `nk_end()` and rolls the number appropriately.
 *
 * also, the crc/hash calculation has to be done, not within
 * `nk_command_buffer_push` but within each individual command type function 
 * - `nk_push_scissor()
 * - `nk_stroke_line()`
 * - `nk_push_image()`
 * - ...
 *   because the `nk_command_buffer_push` actually doesn't _push_ anything. its
 *   really just and allocator that return the pointer, then allows the calling
 *   function to fill that memory space with the parameters. So we need the
 *   individual calling functions to calculate the crc less we lose unique data
 *   for the specific call (looking at you highlight on hover feature).
 * \addtogroup crc 
 * \brief Command buffer CRC Implementation
 * @{
 */

#ifdef NK_DRAW_CRC_CUSTOM
  #ifndef NK_CRC_SEED
    #error "Must #define NK_CRC_SEED when using NK_DRAW_CRC_CUSTOM"
  #endif
  #ifndef NK_CRC_FUNC(k,l,s)
    #error "Must #define NK_CRC_FUNC(key,length,seed) when using NK_DRAW_CRC_CUSTOM"
  #endif
  #define NK_DRAW_CRC
#endif

#ifdef NK_DRAW_CRC
  #define NK_CRC_SEED 0xffffffff /**< seed value of the crc*/
  #define NK_CRC_FUNC(k,l,s) nk_murmur_hash(k,(l), s) 
  #warn "YO FUCK THIS CRC SHIT"

/** @} *//*end documentation grouping*/
#endif /* NK_DRAW_CRC
