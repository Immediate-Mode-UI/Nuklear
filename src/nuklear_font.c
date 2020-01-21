#include "nuklear.h"
#include "nuklear_internal.h"

#ifdef NK_INCLUDE_FONT_BAKING
/* -------------------------------------------------------------
 *
 *                          RECT PACK
 *
 * --------------------------------------------------------------*/
// stb_rect_pack.h - v1.00 - public domain - rectangle packing
// Sean Barrett 2014
//
// Useful for e.g. packing rectangular textures into an atlas.
// Does not do rotation.
//
// Not necessarily the awesomest packing method, but better than
// the totally naive one in stb_truetype (which is primarily what
// this is meant to replace).
//
// Has only had a few tests run, may have issues.
//
// More docs to come.
//
// No memory allocations; uses qsort() and assert() from stdlib.
// Can override those by defining NK_RP_SORT and NK_RP_ASSERT.
//
// This library currently uses the Skyline Bottom-Left algorithm.
//
// Please note: better rectangle packers are welcome! Please
// implement them to the same API, but with a different init
// function.
//
// Credits
//
//  Library
//    Sean Barrett
//  Minor features
//    Martins Mozeiko
//    github:IntellectualKitty
//    
//  Bugfixes / warning fixes
//    Jeremy Jaussaud
//    Fabian Giesen
//
// Version history:
//
//     1.00  (2019-02-25)  avoid small space waste; gracefully fail too-wide rectangles
//     0.99  (2019-02-07)  warning fixes
//     0.11  (2017-03-03)  return packing success/fail result
//     0.10  (2016-10-25)  remove cast-away-const to avoid warnings
//     0.09  (2016-08-27)  fix compiler warnings
//     0.08  (2015-09-13)  really fix bug with empty rects (w=0 or h=0)
//     0.07  (2015-09-13)  fix bug with empty rects (w=0 or h=0)
//     0.06  (2015-04-15)  added NK_RP_SORT to allow replacing qsort
//     0.05:  added NK_RP_ASSERT to allow replacing assert
//     0.04:  fixed minor bug in NK_RP_LARGE_RECTS support
//     0.01:  initial release
//
// LICENSE
//
//   See end of file for license information.

//////////////////////////////////////////////////////////////////////////////
//
//       INCLUDE SECTION
//

#ifndef STB_INCLUDE_STB_RECT_PACK_H
#define STB_INCLUDE_STB_RECT_PACK_H

#define STB_RECT_PACK_VERSION  1

#ifdef NK_RP_STATIC
#define NK_RP_DEF static
#else
#define NK_RP_DEF extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nk_rp_context nk_rp_context;
typedef struct nk_rp_node    nk_rp_node;
typedef struct nk_rp_rect    nk_rp_rect;

#ifdef NK_RP_LARGE_RECTS
typedef int            nk_rp_coord;
#else
typedef unsigned short nk_rp_coord;
#endif

NK_RP_DEF int nk_rp_pack_rects (nk_rp_context *context, nk_rp_rect *rects, int num_rects);
// Assign packed locations to rectangles. The rectangles are of type
// 'nk_rp_rect' defined below, stored in the array 'rects', and there
// are 'num_rects' many of them.
//
// Rectangles which are successfully packed have the 'was_packed' flag
// set to a non-zero value and 'x' and 'y' store the minimum location
// on each axis (i.e. bottom-left in cartesian coordinates, top-left
// if you imagine y increasing downwards). Rectangles which do not fit
// have the 'was_packed' flag set to 0.
//
// You should not try to access the 'rects' array from another thread
// while this function is running, as the function temporarily reorders
// the array while it executes.
//
// To pack into another rectangle, you need to call nk_rp_init_target
// again. To continue packing into the same rectangle, you can call
// this function again. Calling this multiple times with multiple rect
// arrays will probably produce worse packing results than calling it
// a single time with the full rectangle array, but the option is
// available.
//
// The function returns 1 if all of the rectangles were successfully
// packed and 0 otherwise.

struct nk_rp_rect
{
   // reserved for your use:
   int            id;

   // input:
   nk_rp_coord    w, h;

   // output:
   nk_rp_coord    x, y;
   int            was_packed;  // non-zero if valid packing

}; // 16 bytes, nominally


NK_RP_DEF void nk_rp_init_target (nk_rp_context *context, int width, int height, nk_rp_node *nodes, int num_nodes);
// Initialize a rectangle packer to:
//    pack a rectangle that is 'width' by 'height' in dimensions
//    using temporary storage provided by the array 'nodes', which is 'num_nodes' long
//
// You must call this function every time you start packing into a new target.
//
// There is no "shutdown" function. The 'nodes' memory must stay valid for
// the following nk_rp_pack_rects() call (or calls), but can be freed after
// the call (or calls) finish.
//
// Note: to guarantee best results, either:
//       1. make sure 'num_nodes' >= 'width'
//   or  2. call nk_rp_allow_out_of_mem() defined below with 'allow_out_of_mem = 1'
//
// If you don't do either of the above things, widths will be quantized to multiples
// of small integers to guarantee the algorithm doesn't run out of temporary storage.
//
// If you do #2, then the non-quantized algorithm will be used, but the algorithm
// may run out of temporary storage and be unable to pack some rectangles.

NK_RP_DEF void nk_rp_setup_allow_out_of_mem (nk_rp_context *context, int allow_out_of_mem);
// Optionally call this function after init but before doing any packing to
// change the handling of the out-of-temp-memory scenario, described above.
// If you call init again, this will be reset to the default (false).


NK_RP_DEF void nk_rp_setup_heuristic (nk_rp_context *context, int heuristic);
// Optionally select which packing heuristic the library should use. Different
// heuristics will produce better/worse results for different data sets.
// If you call init again, this will be reset to the default.

enum
{
   NK_RP_HEURISTIC_Skyline_default=0,
   NK_RP_HEURISTIC_Skyline_BL_sortHeight = NK_RP_HEURISTIC_Skyline_default,
   NK_RP_HEURISTIC_Skyline_BF_sortHeight
};


//////////////////////////////////////////////////////////////////////////////
//
// the details of the following structures don't matter to you, but they must
// be visible so you can handle the memory allocations for them

struct nk_rp_node
{
   nk_rp_coord  x,y;
   nk_rp_node  *next;
};

struct nk_rp_context
{
   int width;
   int height;
   int align;
   int init_mode;
   int heuristic;
   int num_nodes;
   nk_rp_node *active_head;
   nk_rp_node *free_head;
   nk_rp_node extra[2]; // we allocate two extra nodes so optimal user-node-count is 'width' not 'width+2'
};

#ifdef __cplusplus
}
#endif

#endif

//////////////////////////////////////////////////////////////////////////////
//
//     IMPLEMENTATION SECTION
//

#if 1 //def STB_RECT_PACK_IMPLEMENTATION
#ifndef NK_RP_SORT
#include <stdlib.h>
#define NK_RP_SORT qsort
#endif

#ifndef NK_RP_ASSERT
#include <assert.h>
#define NK_RP_ASSERT assert
#endif

#ifdef _MSC_VER
#define NK_RP__NOTUSED(v)  (void)(v)
#else
#define NK_RP__NOTUSED(v)  (void)sizeof(v)
#endif

enum
{
   NK_RP__INIT_skyline = 1
};

NK_RP_DEF void nk_rp_setup_heuristic(nk_rp_context *context, int heuristic)
{
   switch (context->init_mode) {
      case NK_RP__INIT_skyline:
         NK_RP_ASSERT(heuristic == NK_RP_HEURISTIC_Skyline_BL_sortHeight || heuristic == NK_RP_HEURISTIC_Skyline_BF_sortHeight);
         context->heuristic = heuristic;
         break;
      default:
         NK_RP_ASSERT(0);
   }
}

NK_RP_DEF void nk_rp_setup_allow_out_of_mem(nk_rp_context *context, int allow_out_of_mem)
{
   if (allow_out_of_mem)
      // if it's ok to run out of memory, then don't bother aligning them;
      // this gives better packing, but may fail due to OOM (even though
      // the rectangles easily fit). @TODO a smarter approach would be to only
      // quantize once we've hit OOM, then we could get rid of this parameter.
      context->align = 1;
   else {
      // if it's not ok to run out of memory, then quantize the widths
      // so that num_nodes is always enough nodes.
      //
      // I.e. num_nodes * align >= width
      //                  align >= width / num_nodes
      //                  align = ceil(width/num_nodes)

      context->align = (context->width + context->num_nodes-1) / context->num_nodes;
   }
}

NK_RP_DEF void nk_rp_init_target(nk_rp_context *context, int width, int height, nk_rp_node *nodes, int num_nodes)
{
   int i;
#ifndef NK_RP_LARGE_RECTS
   NK_RP_ASSERT(width <= 0xffff && height <= 0xffff);
#endif

   for (i=0; i < num_nodes-1; ++i)
      nodes[i].next = &nodes[i+1];
   nodes[i].next = NULL;
   context->init_mode = NK_RP__INIT_skyline;
   context->heuristic = NK_RP_HEURISTIC_Skyline_default;
   context->free_head = &nodes[0];
   context->active_head = &context->extra[0];
   context->width = width;
   context->height = height;
   context->num_nodes = num_nodes;
   nk_rp_setup_allow_out_of_mem(context, 0);

   // node 0 is the full width, node 1 is the sentinel (lets us not store width explicitly)
   context->extra[0].x = 0;
   context->extra[0].y = 0;
   context->extra[0].next = &context->extra[1];
   context->extra[1].x = (nk_rp_coord) width;
#ifdef NK_RP_LARGE_RECTS
   context->extra[1].y = (1<<30);
#else
   context->extra[1].y = 65535;
#endif
   context->extra[1].next = NULL;
}

// find minimum y position if it starts at x1
static int nk_rp__skyline_find_min_y(nk_rp_context *c, nk_rp_node *first, int x0, int width, int *pwaste)
{
   nk_rp_node *node = first;
   int x1 = x0 + width;
   int min_y, visited_width, waste_area;

   NK_RP__NOTUSED(c);

   NK_RP_ASSERT(first->x <= x0);

   #if 0
   // skip in case we're past the node
   while (node->next->x <= x0)
      ++node;
   #else
   NK_RP_ASSERT(node->next->x > x0); // we ended up handling this in the caller for efficiency
   #endif

   NK_RP_ASSERT(node->x <= x0);

   min_y = 0;
   waste_area = 0;
   visited_width = 0;
   while (node->x < x1) {
      if (node->y > min_y) {
         // raise min_y higher.
         // we've accounted for all waste up to min_y,
         // but we'll now add more waste for everything we've visted
         waste_area += visited_width * (node->y - min_y);
         min_y = node->y;
         // the first time through, visited_width might be reduced
         if (node->x < x0)
            visited_width += node->next->x - x0;
         else
            visited_width += node->next->x - node->x;
      } else {
         // add waste area
         int under_width = node->next->x - node->x;
         if (under_width + visited_width > width)
            under_width = width - visited_width;
         waste_area += under_width * (min_y - node->y);
         visited_width += under_width;
      }
      node = node->next;
   }

   *pwaste = waste_area;
   return min_y;
}

typedef struct
{
   int x,y;
   nk_rp_node **prev_link;
} nk_rp__findresult;

static nk_rp__findresult nk_rp__skyline_find_best_pos(nk_rp_context *c, int width, int height)
{
   int best_waste = (1<<30), best_x, best_y = (1 << 30);
   nk_rp__findresult fr;
   nk_rp_node **prev, *node, *tail, **best = NULL;

   // align to multiple of c->align
   width = (width + c->align - 1);
   width -= width % c->align;
   NK_RP_ASSERT(width % c->align == 0);

   // if it can't possibly fit, bail immediately
   if (width > c->width || height > c->height) {
      fr.prev_link = NULL;
      fr.x = fr.y = 0;
      return fr;
   }

   node = c->active_head;
   prev = &c->active_head;
   while (node->x + width <= c->width) {
      int y,waste;
      y = nk_rp__skyline_find_min_y(c, node, node->x, width, &waste);
      if (c->heuristic == NK_RP_HEURISTIC_Skyline_BL_sortHeight) { // actually just want to test BL
         // bottom left
         if (y < best_y) {
            best_y = y;
            best = prev;
         }
      } else {
         // best-fit
         if (y + height <= c->height) {
            // can only use it if it first vertically
            if (y < best_y || (y == best_y && waste < best_waste)) {
               best_y = y;
               best_waste = waste;
               best = prev;
            }
         }
      }
      prev = &node->next;
      node = node->next;
   }

   best_x = (best == NULL) ? 0 : (*best)->x;

   // if doing best-fit (BF), we also have to try aligning right edge to each node position
   //
   // e.g, if fitting
   //
   //     ____________________
   //    |____________________|
   //
   //            into
   //
   //   |                         |
   //   |             ____________|
   //   |____________|
   //
   // then right-aligned reduces waste, but bottom-left BL is always chooses left-aligned
   //
   // This makes BF take about 2x the time

   if (c->heuristic == NK_RP_HEURISTIC_Skyline_BF_sortHeight) {
      tail = c->active_head;
      node = c->active_head;
      prev = &c->active_head;
      // find first node that's admissible
      while (tail->x < width)
         tail = tail->next;
      while (tail) {
         int xpos = tail->x - width;
         int y,waste;
         NK_RP_ASSERT(xpos >= 0);
         // find the left position that matches this
         while (node->next->x <= xpos) {
            prev = &node->next;
            node = node->next;
         }
         NK_RP_ASSERT(node->next->x > xpos && node->x <= xpos);
         y = nk_rp__skyline_find_min_y(c, node, xpos, width, &waste);
         if (y + height <= c->height) {
            if (y <= best_y) {
               if (y < best_y || waste < best_waste || (waste==best_waste && xpos < best_x)) {
                  best_x = xpos;
                  NK_RP_ASSERT(y <= best_y);
                  best_y = y;
                  best_waste = waste;
                  best = prev;
               }
            }
         }
         tail = tail->next;
      }         
   }

   fr.prev_link = best;
   fr.x = best_x;
   fr.y = best_y;
   return fr;
}

static nk_rp__findresult nk_rp__skyline_pack_rectangle(nk_rp_context *context, int width, int height)
{
   // find best position according to heuristic
   nk_rp__findresult res = nk_rp__skyline_find_best_pos(context, width, height);
   nk_rp_node *node, *cur;

   // bail if:
   //    1. it failed
   //    2. the best node doesn't fit (we don't always check this)
   //    3. we're out of memory
   if (res.prev_link == NULL || res.y + height > context->height || context->free_head == NULL) {
      res.prev_link = NULL;
      return res;
   }

   // on success, create new node
   node = context->free_head;
   node->x = (nk_rp_coord) res.x;
   node->y = (nk_rp_coord) (res.y + height);

   context->free_head = node->next;

   // insert the new node into the right starting point, and
   // let 'cur' point to the remaining nodes needing to be
   // stiched back in

   cur = *res.prev_link;
   if (cur->x < res.x) {
      // preserve the existing one, so start testing with the next one
      nk_rp_node *next = cur->next;
      cur->next = node;
      cur = next;
   } else {
      *res.prev_link = node;
   }

   // from here, traverse cur and free the nodes, until we get to one
   // that shouldn't be freed
   while (cur->next && cur->next->x <= res.x + width) {
      nk_rp_node *next = cur->next;
      // move the current node to the free list
      cur->next = context->free_head;
      context->free_head = cur;
      cur = next;
   }

   // stitch the list back in
   node->next = cur;

   if (cur->x < res.x + width)
      cur->x = (nk_rp_coord) (res.x + width);

#ifdef _DEBUG
   cur = context->active_head;
   while (cur->x < context->width) {
      NK_RP_ASSERT(cur->x < cur->next->x);
      cur = cur->next;
   }
   NK_RP_ASSERT(cur->next == NULL);

   {
      int count=0;
      cur = context->active_head;
      while (cur) {
         cur = cur->next;
         ++count;
      }
      cur = context->free_head;
      while (cur) {
         cur = cur->next;
         ++count;
      }
      NK_RP_ASSERT(count == context->num_nodes+2);
   }
#endif

   return res;
}

static int rect_height_compare(const void *a, const void *b)
{
   const nk_rp_rect *p = (const nk_rp_rect *) a;
   const nk_rp_rect *q = (const nk_rp_rect *) b;
   if (p->h > q->h)
      return -1;
   if (p->h < q->h)
      return  1;
   return (p->w > q->w) ? -1 : (p->w < q->w);
}

static int rect_original_order(const void *a, const void *b)
{
   const nk_rp_rect *p = (const nk_rp_rect *) a;
   const nk_rp_rect *q = (const nk_rp_rect *) b;
   return (p->was_packed < q->was_packed) ? -1 : (p->was_packed > q->was_packed);
}

#ifdef NK_RP_LARGE_RECTS
#define NK_RP__MAXVAL  0xffffffff
#else
#define NK_RP__MAXVAL  0xffff
#endif

NK_RP_DEF int nk_rp_pack_rects(nk_rp_context *context, nk_rp_rect *rects, int num_rects)
{
   int i, all_rects_packed = 1;

   // we use the 'was_packed' field internally to allow sorting/unsorting
   for (i=0; i < num_rects; ++i) {
      rects[i].was_packed = i;
   }

   // sort according to heuristic
   NK_RP_SORT(rects, num_rects, sizeof(rects[0]), rect_height_compare);

   for (i=0; i < num_rects; ++i) {
      if (rects[i].w == 0 || rects[i].h == 0) {
         rects[i].x = rects[i].y = 0;  // empty rect needs no space
      } else {
         nk_rp__findresult fr = nk_rp__skyline_pack_rectangle(context, rects[i].w, rects[i].h);
         if (fr.prev_link) {
            rects[i].x = (nk_rp_coord) fr.x;
            rects[i].y = (nk_rp_coord) fr.y;
         } else {
            rects[i].x = rects[i].y = NK_RP__MAXVAL;
         }
      }
   }

   // unsort
   NK_RP_SORT(rects, num_rects, sizeof(rects[0]), rect_original_order);

   // set was_packed flags and all_rects_packed status
   for (i=0; i < num_rects; ++i) {
      rects[i].was_packed = !(rects[i].x == NK_RP__MAXVAL && rects[i].y == NK_RP__MAXVAL);
      if (!rects[i].was_packed)
         all_rects_packed = 0;
   }

   // return the all_rects_packed status
   return all_rects_packed;
}
#endif

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this 
software, either in source code form or as a compiled binary, for any purpose, 
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this 
software dedicate any and all copyright interest in the software to the public 
domain. We make this dedication for the benefit of the public at large and to 
the detriment of our heirs and successors. We intend this dedication to be an 
overt act of relinquishment in perpetuity of all present and future rights to 
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/

// stb_truetype.h - v1.22 - public domain
// authored from 2009-2019 by Sean Barrett / RAD Game Tools
//
//   This library processes TrueType files:
//        parse files
//        extract glyph metrics
//        extract glyph shapes
//        render glyphs to one-channel bitmaps with antialiasing (box filter)
//        render glyphs to one-channel SDF bitmaps (signed-distance field/function)
//
//   Todo:
//        non-MS cmaps
//        crashproof on bad data
//        hinting? (no longer patented)
//        cleartype-style AA?
//        optimize: use simple memory allocator for intermediates
//        optimize: build edge-list directly from curves
//        optimize: rasterize directly from curves?
//
// ADDITIONAL CONTRIBUTORS
//
//   Mikko Mononen: compound shape support, more cmap formats
//   Tor Andersson: kerning, subpixel rendering
//   Dougall Johnson: OpenType / Type 2 font handling
//   Daniel Ribeiro Maciel: basic GPOS-based kerning
//
//   Misc other:
//       Ryan Gordon
//       Simon Glass
//       github:IntellectualKitty
//       Imanol Celaya
//       Daniel Ribeiro Maciel
//
//   Bug/warning reports/fixes:
//       "Zer" on mollyrocket       Fabian "ryg" Giesen
//       Cass Everitt               Martins Mozeiko
//       stoiko (Haemimont Games)   Cap Petschulat
//       Brian Hook                 Omar Cornut
//       Walter van Niftrik         github:aloucks
//       David Gow                  Peter LaValle
//       David Given                Sergey Popov
//       Ivan-Assen Ivanov          Giumo X. Clanjor
//       Anthony Pesch              Higor Euripedes
//       Johan Duparc               Thomas Fields
//       Hou Qiming                 Derek Vinyard
//       Rob Loach                  Cort Stratton
//       Kenney Phillis Jr.         github:oyvindjam
//       Brian Costabile            github:vassvik
//       Ken Voskuil (kaesve)       Ryan Griege
//       
// VERSION HISTORY
//
//   1.22 (2019-08-11) minimize missing-glyph duplication; fix kerning if both 'GPOS' and 'kern' are defined 
//   1.21 (2019-02-25) fix warning
//   1.20 (2019-02-07) PackFontRange skips missing codepoints; GetScaleFontVMetrics()
//   1.19 (2018-02-11) GPOS kerning, NK_TT_fmod
//   1.18 (2018-01-29) add missing function
//   1.17 (2017-07-23) make more arguments const; doc fix
//   1.16 (2017-07-12) SDF support
//   1.15 (2017-03-03) make more arguments const
//   1.14 (2017-01-16) num-fonts-in-TTC function
//   1.13 (2017-01-02) support OpenType fonts, certain Apple fonts
//   1.12 (2016-10-25) suppress warnings about casting away const with -Wcast-qual
//   1.11 (2016-04-02) fix unused-variable warning
//   1.10 (2016-04-02) user-defined fabs(); rare memory leak; remove duplicate typedef
//   1.09 (2016-01-16) warning fix; avoid crash on outofmem; use allocation userdata properly
//   1.08 (2015-09-13) document nk_tt_Rasterize(); fixes for vertical & horizontal edges
//   1.07 (2015-08-01) allow PackFontRanges to accept arrays of sparse codepoints;
//                     variant PackFontRanges to pack and render in separate phases;
//                     fix nk_tt_GetFontOFfsetForIndex (never worked for non-0 input?);
//                     fixed an assert() bug in the new rasterizer
//                     replace assert() with NK_TT_assert() in new rasterizer
//
//   Full history can be found at the end of this file.
//
// LICENSE
//
//   See end of file for license information.
//
// USAGE
//
//   Include this file in whatever places need to refer to it. In ONE C/C++
//   file, write:
//      #define STB_TRUETYPE_IMPLEMENTATION
//   before the #include of this file. This expands out the actual
//   implementation into that C/C++ file.
//
//   To make the implementation private to the file that generates the implementation,
//      #define NK_TT_STATIC
//
//   Simple 3D API (don't ship this, but it's fine for tools and quick start)
//           nk_tt_BakeFontBitmap()               -- bake a font to a bitmap for use as texture
//           nk_tt_GetBakedQuad()                 -- compute quad to draw for a given char
//
//   Improved 3D API (more shippable):
//           #include "stb_rect_pack.h"           -- optional, but you really want it
//           nk_tt_PackBegin()
//           nk_tt_PackSetOversampling()          -- for improved quality on small fonts
//           nk_tt_PackFontRanges()               -- pack and renders
//           nk_tt_PackEnd()
//           nk_tt_GetPackedQuad()
//
//   "Load" a font file from a memory buffer (you have to keep the buffer loaded)
//           nk_tt_InitFont()
//           nk_tt_GetFontOffsetForIndex()        -- indexing for TTC font collections
//           nk_tt_GetNumberOfFonts()             -- number of fonts for TTC font collections
//
//   Render a unicode codepoint to a bitmap
//           nk_tt_GetCodepointBitmap()           -- allocates and returns a bitmap
//           nk_tt_MakeCodepointBitmap()          -- renders into bitmap you provide
//           nk_tt_GetCodepointBitmapBox()        -- how big the bitmap must be
//
//   Character advance/positioning
//           nk_tt_GetCodepointHMetrics()
//           nk_tt_GetFontVMetrics()
//           nk_tt_GetFontVMetricsOS2()
//           nk_tt_GetCodepointKernAdvance()
//
//   Starting with version 1.06, the rasterizer was replaced with a new,
//   faster and generally-more-precise rasterizer. The new rasterizer more
//   accurately measures pixel coverage for anti-aliasing, except in the case
//   where multiple shapes overlap, in which case it overestimates the AA pixel
//   coverage. Thus, anti-aliasing of intersecting shapes may look wrong. If
//   this turns out to be a problem, you can re-enable the old rasterizer with
//        #define NK_TT_RASTERIZER_VERSION 1
//   which will incur about a 15% speed hit.
//
// ADDITIONAL DOCUMENTATION
//
//   Immediately after this block comment are a series of sample programs.
//
//   After the sample programs is the "header file" section. This section
//   includes documentation for each API function.
//
//   Some important concepts to understand to use this library:
//
//      Codepoint
//         Characters are defined by unicode codepoints, e.g. 65 is
//         uppercase A, 231 is lowercase c with a cedilla, 0x7e30 is
//         the hiragana for "ma".
//
//      Glyph
//         A visual character shape (every codepoint is rendered as
//         some glyph)
//
//      Glyph index
//         A font-specific integer ID representing a glyph
//
//      Baseline
//         Glyph shapes are defined relative to a baseline, which is the
//         bottom of uppercase characters. Characters extend both above
//         and below the baseline.
//
//      Current Point
//         As you draw text to the screen, you keep track of a "current point"
//         which is the origin of each character. The current point's vertical
//         position is the baseline. Even "baked fonts" use this model.
//
//      Vertical Font Metrics
//         The vertical qualities of the font, used to vertically position
//         and space the characters. See docs for nk_tt_GetFontVMetrics.
//
//      Font Size in Pixels or Points
//         The preferred interface for specifying font sizes in stb_truetype
//         is to specify how tall the font's vertical extent should be in pixels.
//         If that sounds good enough, skip the next paragraph.
//
//         Most font APIs instead use "points", which are a common typographic
//         measurement for describing font size, defined as 72 points per inch.
//         stb_truetype provides a point API for compatibility. However, true
//         "per inch" conventions don't make much sense on computer displays
//         since different monitors have different number of pixels per
//         inch. For example, Windows traditionally uses a convention that
//         there are 96 pixels per inch, thus making 'inch' measurements have
//         nothing to do with inches, and thus effectively defining a point to
//         be 1.333 pixels. Additionally, the TrueType font data provides
//         an explicit scale factor to scale a given font's glyphs to points,
//         but the author has observed that this scale factor is often wrong
//         for non-commercial fonts, thus making fonts scaled in points
//         according to the TrueType spec incoherently sized in practice.
//
// DETAILED USAGE:
//
//  Scale:
//    Select how high you want the font to be, in points or pixels.
//    Call ScaleForPixelHeight or ScaleForMappingEmToPixels to compute
//    a scale factor SF that will be used by all other functions.
//
//  Baseline:
//    You need to select a y-coordinate that is the baseline of where
//    your text will appear. Call GetFontBoundingBox to get the baseline-relative
//    bounding box for all characters. SF*-y0 will be the distance in pixels
//    that the worst-case character could extend above the baseline, so if
//    you want the top edge of characters to appear at the top of the
//    screen where y=0, then you would set the baseline to SF*-y0.
//
//  Current point:
//    Set the current point where the first character will appear. The
//    first character could extend left of the current point; this is font
//    dependent. You can either choose a current point that is the leftmost
//    point and hope, or add some padding, or check the bounding box or
//    left-side-bearing of the first character to be displayed and set
//    the current point based on that.
//
//  Displaying a character:
//    Compute the bounding box of the character. It will contain signed values
//    relative to <current_point, baseline>. I.e. if it returns x0,y0,x1,y1,
//    then the character should be displayed in the rectangle from
//    <current_point+SF*x0, baseline+SF*y0> to <current_point+SF*x1,baseline+SF*y1).
//
//  Advancing for the next character:
//    Call GlyphHMetrics, and compute 'current_point += SF * advance'.
// 
//
// ADVANCED USAGE
//
//   Quality:
//
//    - Use the functions with Subpixel at the end to allow your characters
//      to have subpixel positioning. Since the font is anti-aliased, not
//      hinted, this is very import for quality. (This is not possible with
//      baked fonts.)
//
//    - Kerning is now supported, and if you're supporting subpixel rendering
//      then kerning is worth using to give your text a polished look.
//
//   Performance:
//
//    - Convert Unicode codepoints to glyph indexes and operate on the glyphs;
//      if you don't do this, stb_truetype is forced to do the conversion on
//      every call.
//
//    - There are a lot of memory allocations. We should modify it to take
//      a temp buffer and allocate from the temp buffer (without freeing),
//      should help performance a lot.
//
// NOTES
//
//   The system uses the raw data found in the .ttf file without changing it
//   and without building auxiliary data structures. This is a bit inefficient
//   on little-endian systems (the data is big-endian), but assuming you're
//   caching the bitmaps or glyph shapes this shouldn't be a big deal.
//
//   It appears to be very hard to programmatically determine what font a
//   given file is in a general way. I provide an API for this, but I don't
//   recommend it.
//
//
// PERFORMANCE MEASUREMENTS FOR 1.06:
//
//                      32-bit     64-bit
//   Previous release:  8.83 s     7.68 s
//   Pool allocations:  7.72 s     6.34 s
//   Inline sort     :  6.54 s     5.65 s
//   New rasterizer  :  5.63 s     5.00 s

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
////
////   INTEGRATION WITH YOUR CODEBASE
////
////   The following sections allow you to supply alternate definitions
////   of C library functions used by stb_truetype, e.g. if you don't
////   link with the C runtime library.

#if 1 // def STB_TRUETYPE_IMPLEMENTATION
   // #define your own (u)nk_tt_int8/16/32 before including to override this
   #ifndef nk_tt_uint8
   typedef unsigned char   nk_tt_uint8;
   typedef signed   char   nk_tt_int8;
   typedef unsigned short  nk_tt_uint16;
   typedef signed   short  nk_tt_int16;
   typedef unsigned int    nk_tt_uint32;
   typedef signed   int    nk_tt_int32;
   #endif

   typedef char nk_tt__check_size32[sizeof(nk_tt_int32)==4 ? 1 : -1];
   typedef char nk_tt__check_size16[sizeof(nk_tt_int16)==2 ? 1 : -1];

   // e.g. #define your own NK_TT_ifloor/NK_TT_iceil() to avoid math.h
   #ifndef NK_TT_ifloor
   #include <math.h>
   #define NK_TT_ifloor(x)   ((int) floor(x))
   #define NK_TT_iceil(x)    ((int) ceil(x))
   #endif

   #ifndef NK_TT_sqrt
   #include <math.h>
   #define NK_TT_sqrt(x)      sqrt(x)
   #define NK_TT_pow(x,y)     pow(x,y)
   #endif

   #ifndef NK_TT_fmod
   #include <math.h>
   #define NK_TT_fmod(x,y)    fmod(x,y)
   #endif

   #ifndef NK_TT_cos
   #include <math.h>
   #define NK_TT_cos(x)       cos(x)
   #define NK_TT_acos(x)      acos(x)
   #endif

   #ifndef NK_TT_fabs
   #include <math.h>
   #define NK_TT_fabs(x)      fabs(x)
   #endif

   // #define your own functions "NK_TT_malloc" / "NK_TT_free" to avoid malloc.h
   #ifndef NK_TT_malloc
   #include <stdlib.h>
   #define NK_TT_malloc(x,u)  ((void)(u),malloc(x))
   #define NK_TT_free(x,u)    ((void)(u),free(x))
   #endif

   #ifndef NK_TT_assert
   #include <assert.h>
   #define NK_TT_assert(x)    assert(x)
   #endif

   #ifndef NK_TT_strlen
   #include <string.h>
   #define NK_TT_strlen(x)    strlen(x)
   #endif

   #ifndef NK_TT_memcpy
   #include <string.h>
   #define NK_TT_memcpy       memcpy
   #define NK_TT_memset       memset
   #endif
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////   INTERFACE
////
////

#ifndef __STB_INCLUDE_STB_TRUETYPE_H__
#define __STB_INCLUDE_STB_TRUETYPE_H__

#ifdef NK_TT_STATIC
#define NK_TT_DEF static
#else
#define NK_TT_DEF extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

// private structure
typedef struct
{
   unsigned char *data;
   int cursor;
   int size;
} nk_tt__buf;

//////////////////////////////////////////////////////////////////////////////
//
// TEXTURE BAKING API
//
// If you use this API, you only have to call two functions ever.
//

typedef struct
{
   unsigned short x0,y0,x1,y1; // coordinates of bbox in bitmap
   float xoff,yoff,xadvance;
} nk_tt_bakedchar;

NK_TT_DEF int nk_tt_BakeFontBitmap(const unsigned char *data, int offset,  // font location (use offset=0 for plain .ttf)
                                float pixel_height,                     // height of font in pixels
                                unsigned char *pixels, int pw, int ph,  // bitmap to be filled in
                                int first_char, int num_chars,          // characters to bake
                                nk_tt_bakedchar *chardata);             // you allocate this, it's num_chars long
// if return is positive, the first unused row of the bitmap
// if return is negative, returns the negative of the number of characters that fit
// if return is 0, no characters fit and no rows were used
// This uses a very crappy packing.

typedef struct
{
   float x0,y0,s0,t0; // top-left
   float x1,y1,s1,t1; // bottom-right
} nk_tt_aligned_quad;

NK_TT_DEF void nk_tt_GetBakedQuad(const nk_tt_bakedchar *chardata, int pw, int ph,  // same data as above
                               int char_index,             // character to display
                               float *xpos, float *ypos,   // pointers to current position in screen pixel space
                               nk_tt_aligned_quad *q,      // output: quad to draw
                               int opengl_fillrule);       // true if opengl fill rule; false if DX9 or earlier
// Call GetBakedQuad with char_index = 'character - first_char', and it
// creates the quad you need to draw and advances the current position.
//
// The coordinate system used assumes y increases downwards.
//
// Characters will extend both above and below the current position;
// see discussion of "BASELINE" above.
//
// It's inefficient; you might want to c&p it and optimize it.

NK_TT_DEF void nk_tt_GetScaledFontVMetrics(const unsigned char *fontdata, int index, float size, float *ascent, float *descent, float *lineGap);
// Query the font vertical metrics without having to create a font first.


//////////////////////////////////////////////////////////////////////////////
//
// NEW TEXTURE BAKING API
//
// This provides options for packing multiple fonts into one atlas, not
// perfectly but better than nothing.

typedef struct
{
   unsigned short x0,y0,x1,y1; // coordinates of bbox in bitmap
   float xoff,yoff,xadvance;
   float xoff2,yoff2;
} nk_tt_packedchar;

typedef struct nk_tt_pack_context nk_tt_pack_context;
typedef struct nk_tt_fontinfo nk_tt_fontinfo;
#ifndef STB_RECT_PACK_VERSION
typedef struct nk_rp_rect nk_rp_rect;
#endif

NK_TT_DEF int  nk_tt_PackBegin(nk_tt_pack_context *spc, unsigned char *pixels, int width, int height, int stride_in_bytes, int padding, void *alloc_context);
// Initializes a packing context stored in the passed-in nk_tt_pack_context.
// Future calls using this context will pack characters into the bitmap passed
// in here: a 1-channel bitmap that is width * height. stride_in_bytes is
// the distance from one row to the next (or 0 to mean they are packed tightly
// together). "padding" is the amount of padding to leave between each
// character (normally you want '1' for bitmaps you'll use as textures with
// bilinear filtering).
//
// Returns 0 on failure, 1 on success.

NK_TT_DEF void nk_tt_PackEnd  (nk_tt_pack_context *spc);
// Cleans up the packing context and frees all memory.

#define NK_TT_POINT_SIZE(x)   (-(x))

NK_TT_DEF int  nk_tt_PackFontRange(nk_tt_pack_context *spc, const unsigned char *fontdata, int font_index, float font_size,
                                int first_unicode_char_in_range, int num_chars_in_range, nk_tt_packedchar *chardata_for_range);
// Creates character bitmaps from the font_index'th font found in fontdata (use
// font_index=0 if you don't know what that is). It creates num_chars_in_range
// bitmaps for characters with unicode values starting at first_unicode_char_in_range
// and increasing. Data for how to render them is stored in chardata_for_range;
// pass these to nk_tt_GetPackedQuad to get back renderable quads.
//
// font_size is the full height of the character from ascender to descender,
// as computed by nk_tt_ScaleForPixelHeight. To use a point size as computed
// by nk_tt_ScaleForMappingEmToPixels, wrap the point size in NK_TT_POINT_SIZE()
// and pass that result as 'font_size':
//       ...,                  20 , ... // font max minus min y is 20 pixels tall
//       ..., NK_TT_POINT_SIZE(20), ... // 'M' is 20 pixels tall

typedef struct
{
   float font_size;
   int first_unicode_codepoint_in_range;  // if non-zero, then the chars are continuous, and this is the first codepoint
   int *array_of_unicode_codepoints;       // if non-zero, then this is an array of unicode codepoints
   int num_chars;
   nk_tt_packedchar *chardata_for_range; // output
   unsigned char h_oversample, v_oversample; // don't set these, they're used internally
} nk_tt_pack_range;

NK_TT_DEF int  nk_tt_PackFontRanges(nk_tt_pack_context *spc, const unsigned char *fontdata, int font_index, nk_tt_pack_range *ranges, int num_ranges);
// Creates character bitmaps from multiple ranges of characters stored in
// ranges. This will usually create a better-packed bitmap than multiple
// calls to nk_tt_PackFontRange. Note that you can call this multiple
// times within a single PackBegin/PackEnd.

NK_TT_DEF void nk_tt_PackSetOversampling(nk_tt_pack_context *spc, unsigned int h_oversample, unsigned int v_oversample);
// Oversampling a font increases the quality by allowing higher-quality subpixel
// positioning, and is especially valuable at smaller text sizes.
//
// This function sets the amount of oversampling for all following calls to
// nk_tt_PackFontRange(s) or nk_tt_PackFontRangesGatherRects for a given
// pack context. The default (no oversampling) is achieved by h_oversample=1
// and v_oversample=1. The total number of pixels required is
// h_oversample*v_oversample larger than the default; for example, 2x2
// oversampling requires 4x the storage of 1x1. For best results, render
// oversampled textures with bilinear filtering. Look at the readme in
// stb/tests/oversample for information about oversampled fonts
//
// To use with PackFontRangesGather etc., you must set it before calls
// call to PackFontRangesGatherRects.

NK_TT_DEF void nk_tt_PackSetSkipMissingCodepoints(nk_tt_pack_context *spc, int skip);
// If skip != 0, this tells stb_truetype to skip any codepoints for which
// there is no corresponding glyph. If skip=0, which is the default, then
// codepoints without a glyph recived the font's "missing character" glyph,
// typically an empty box by convention.

NK_TT_DEF void nk_tt_GetPackedQuad(const nk_tt_packedchar *chardata, int pw, int ph,  // same data as above
                               int char_index,             // character to display
                               float *xpos, float *ypos,   // pointers to current position in screen pixel space
                               nk_tt_aligned_quad *q,      // output: quad to draw
                               int align_to_integer);

NK_TT_DEF int  nk_tt_PackFontRangesGatherRects(nk_tt_pack_context *spc, const nk_tt_fontinfo *info, nk_tt_pack_range *ranges, int num_ranges, nk_rp_rect *rects);
NK_TT_DEF void nk_tt_PackFontRangesPackRects(nk_tt_pack_context *spc, nk_rp_rect *rects, int num_rects);
NK_TT_DEF int  nk_tt_PackFontRangesRenderIntoRects(nk_tt_pack_context *spc, const nk_tt_fontinfo *info, nk_tt_pack_range *ranges, int num_ranges, nk_rp_rect *rects);
// Calling these functions in sequence is roughly equivalent to calling
// nk_tt_PackFontRanges(). If you more control over the packing of multiple
// fonts, or if you want to pack custom data into a font texture, take a look
// at the source to of nk_tt_PackFontRanges() and create a custom version 
// using these functions, e.g. call GatherRects multiple times,
// building up a single array of rects, then call PackRects once,
// then call RenderIntoRects repeatedly. This may result in a
// better packing than calling PackFontRanges multiple times
// (or it may not).

// this is an opaque structure that you shouldn't mess with which holds
// all the context needed from PackBegin to PackEnd.
struct nk_tt_pack_context {
   void *user_allocator_context;
   void *pack_info;
   int   width;
   int   height;
   int   stride_in_bytes;
   int   padding;
   int   skip_missing;
   unsigned int   h_oversample, v_oversample;
   unsigned char *pixels;
   void  *nodes;
};

//////////////////////////////////////////////////////////////////////////////
//
// FONT LOADING
//
//

NK_TT_DEF int nk_tt_GetNumberOfFonts(const unsigned char *data);
// This function will determine the number of fonts in a font file.  TrueType
// collection (.ttc) files may contain multiple fonts, while TrueType font
// (.ttf) files only contain one font. The number of fonts can be used for
// indexing with the previous function where the index is between zero and one
// less than the total fonts. If an error occurs, -1 is returned.

NK_TT_DEF int nk_tt_GetFontOffsetForIndex(const unsigned char *data, int index);
// Each .ttf/.ttc file may have more than one font. Each font has a sequential
// index number starting from 0. Call this function to get the font offset for
// a given index; it returns -1 if the index is out of range. A regular .ttf
// file will only define one font and it always be at offset 0, so it will
// return '0' for index 0, and -1 for all other indices.

// The following structure is defined publicly so you can declare one on
// the stack or as a global or etc, but you should treat it as opaque.
struct nk_tt_fontinfo
{
   void           * userdata;
   unsigned char  * data;              // pointer to .ttf file
   int              fontstart;         // offset of start of font

   int numGlyphs;                     // number of glyphs, needed for range checking

   int loca,head,glyf,hhea,hmtx,kern,gpos; // table locations as offset from start of .ttf
   int index_map;                     // a cmap mapping for our chosen character encoding
   int indexToLocFormat;              // format needed to map from glyph index to glyph

   nk_tt__buf cff;                    // cff font data
   nk_tt__buf charstrings;            // the charstring index
   nk_tt__buf gsubrs;                 // global charstring subroutines index
   nk_tt__buf subrs;                  // private charstring subroutines index
   nk_tt__buf fontdicts;              // array of font dicts
   nk_tt__buf fdselect;               // map from glyph to fontdict
};

NK_TT_DEF int nk_tt_InitFont(nk_tt_fontinfo *info, const unsigned char *data, int offset);
// Given an offset into the file that defines a font, this function builds
// the necessary cached info for the rest of the system. You must allocate
// the nk_tt_fontinfo yourself, and nk_tt_InitFont will fill it out. You don't
// need to do anything special to free it, because the contents are pure
// value data with no additional data structures. Returns 0 on failure.


//////////////////////////////////////////////////////////////////////////////
//
// CHARACTER TO GLYPH-INDEX CONVERSIOn

NK_TT_DEF int nk_tt_FindGlyphIndex(const nk_tt_fontinfo *info, int unicode_codepoint);
// If you're going to perform multiple operations on the same character
// and you want a speed-up, call this function with the character you're
// going to process, then use glyph-based functions instead of the
// codepoint-based functions.
// Returns 0 if the character codepoint is not defined in the font.


//////////////////////////////////////////////////////////////////////////////
//
// CHARACTER PROPERTIES
//

NK_TT_DEF float nk_tt_ScaleForPixelHeight(const nk_tt_fontinfo *info, float pixels);
// computes a scale factor to produce a font whose "height" is 'pixels' tall.
// Height is measured as the distance from the highest ascender to the lowest
// descender; in other words, it's equivalent to calling nk_tt_GetFontVMetrics
// and computing:
//       scale = pixels / (ascent - descent)
// so if you prefer to measure height by the ascent only, use a similar calculation.

NK_TT_DEF float nk_tt_ScaleForMappingEmToPixels(const nk_tt_fontinfo *info, float pixels);
// computes a scale factor to produce a font whose EM size is mapped to
// 'pixels' tall. This is probably what traditional APIs compute, but
// I'm not positive.

NK_TT_DEF void nk_tt_GetFontVMetrics(const nk_tt_fontinfo *info, int *ascent, int *descent, int *lineGap);
// ascent is the coordinate above the baseline the font extends; descent
// is the coordinate below the baseline the font extends (i.e. it is typically negative)
// lineGap is the spacing between one row's descent and the next row's ascent...
// so you should advance the vertical position by "*ascent - *descent + *lineGap"
//   these are expressed in unscaled coordinates, so you must multiply by
//   the scale factor for a given size

NK_TT_DEF int  nk_tt_GetFontVMetricsOS2(const nk_tt_fontinfo *info, int *typoAscent, int *typoDescent, int *typoLineGap);
// analogous to GetFontVMetrics, but returns the "typographic" values from the OS/2
// table (specific to MS/Windows TTF files).
//
// Returns 1 on success (table present), 0 on failure.

NK_TT_DEF void nk_tt_GetFontBoundingBox(const nk_tt_fontinfo *info, int *x0, int *y0, int *x1, int *y1);
// the bounding box around all possible characters

NK_TT_DEF void nk_tt_GetCodepointHMetrics(const nk_tt_fontinfo *info, int codepoint, int *advanceWidth, int *leftSideBearing);
// leftSideBearing is the offset from the current horizontal position to the left edge of the character
// advanceWidth is the offset from the current horizontal position to the next horizontal position
//   these are expressed in unscaled coordinates

NK_TT_DEF int  nk_tt_GetCodepointKernAdvance(const nk_tt_fontinfo *info, int ch1, int ch2);
// an additional amount to add to the 'advance' value between ch1 and ch2

NK_TT_DEF int nk_tt_GetCodepointBox(const nk_tt_fontinfo *info, int codepoint, int *x0, int *y0, int *x1, int *y1);
// Gets the bounding box of the visible part of the glyph, in unscaled coordinates

NK_TT_DEF void nk_tt_GetGlyphHMetrics(const nk_tt_fontinfo *info, int glyph_index, int *advanceWidth, int *leftSideBearing);
NK_TT_DEF int  nk_tt_GetGlyphKernAdvance(const nk_tt_fontinfo *info, int glyph1, int glyph2);
NK_TT_DEF int  nk_tt_GetGlyphBox(const nk_tt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1);
// as above, but takes one or more glyph indices for greater efficiency


//////////////////////////////////////////////////////////////////////////////
//
// GLYPH SHAPES (you probably don't need these, but they have to go before
// the bitmaps for C declaration-order reasons)
//

#ifndef NK_TT_vmove // you can predefine these to use different values (but why?)
   enum {
      NK_TT_vmove=1,
      NK_TT_vline,
      NK_TT_vcurve,
      NK_TT_vcubic
   };
#endif

#ifndef nk_tt_vertex // you can predefine this to use different values
                   // (we share this with other code at RAD)
   #define nk_tt_vertex_type short // can't use nk_tt_int16 because that's not visible in the header file
   typedef struct
   {
      nk_tt_vertex_type x,y,cx,cy,cx1,cy1;
      unsigned char type,padding;
   } nk_tt_vertex;
#endif

NK_TT_DEF int nk_tt_IsGlyphEmpty(const nk_tt_fontinfo *info, int glyph_index);
// returns non-zero if nothing is drawn for this glyph

NK_TT_DEF int nk_tt_GetCodepointShape(const nk_tt_fontinfo *info, int unicode_codepoint, nk_tt_vertex **vertices);
NK_TT_DEF int nk_tt_GetGlyphShape(const nk_tt_fontinfo *info, int glyph_index, nk_tt_vertex **vertices);
// returns # of vertices and fills *vertices with the pointer to them
//   these are expressed in "unscaled" coordinates
//
// The shape is a series of contours. Each one starts with
// a NK_TT_moveto, then consists of a series of mixed
// NK_TT_lineto and NK_TT_curveto segments. A lineto
// draws a line from previous endpoint to its x,y; a curveto
// draws a quadratic bezier from previous endpoint to
// its x,y, using cx,cy as the bezier control point.

NK_TT_DEF void nk_tt_FreeShape(const nk_tt_fontinfo *info, nk_tt_vertex *vertices);
// frees the data allocated above

//////////////////////////////////////////////////////////////////////////////
//
// BITMAP RENDERING
//

NK_TT_DEF void nk_tt_FreeBitmap(unsigned char *bitmap, void *userdata);
// frees the bitmap allocated below

NK_TT_DEF unsigned char *nk_tt_GetCodepointBitmap(const nk_tt_fontinfo *info, float scale_x, float scale_y, int codepoint, int *width, int *height, int *xoff, int *yoff);
// allocates a large-enough single-channel 8bpp bitmap and renders the
// specified character/glyph at the specified scale into it, with
// antialiasing. 0 is no coverage (transparent), 255 is fully covered (opaque).
// *width & *height are filled out with the width & height of the bitmap,
// which is stored left-to-right, top-to-bottom.
//
// xoff/yoff are the offset it pixel space from the glyph origin to the top-left of the bitmap

NK_TT_DEF unsigned char *nk_tt_GetCodepointBitmapSubpixel(const nk_tt_fontinfo *info, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint, int *width, int *height, int *xoff, int *yoff);
// the same as nk_tt_GetCodepoitnBitmap, but you can specify a subpixel
// shift for the character

NK_TT_DEF void nk_tt_MakeCodepointBitmap(const nk_tt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int codepoint);
// the same as nk_tt_GetCodepointBitmap, but you pass in storage for the bitmap
// in the form of 'output', with row spacing of 'out_stride' bytes. the bitmap
// is clipped to out_w/out_h bytes. Call nk_tt_GetCodepointBitmapBox to get the
// width and height and positioning info for it first.

NK_TT_DEF void nk_tt_MakeCodepointBitmapSubpixel(const nk_tt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint);
// same as nk_tt_MakeCodepointBitmap, but you can specify a subpixel
// shift for the character

NK_TT_DEF void nk_tt_MakeCodepointBitmapSubpixelPrefilter(const nk_tt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int oversample_x, int oversample_y, float *sub_x, float *sub_y, int codepoint);
// same as nk_tt_MakeCodepointBitmapSubpixel, but prefiltering
// is performed (see nk_tt_PackSetOversampling)

NK_TT_DEF void nk_tt_GetCodepointBitmapBox(const nk_tt_fontinfo *font, int codepoint, float scale_x, float scale_y, int *ix0, int *iy0, int *ix1, int *iy1);
// get the bbox of the bitmap centered around the glyph origin; so the
// bitmap width is ix1-ix0, height is iy1-iy0, and location to place
// the bitmap top left is (leftSideBearing*scale,iy0).
// (Note that the bitmap uses y-increases-down, but the shape uses
// y-increases-up, so CodepointBitmapBox and CodepointBox are inverted.)

NK_TT_DEF void nk_tt_GetCodepointBitmapBoxSubpixel(const nk_tt_fontinfo *font, int codepoint, float scale_x, float scale_y, float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1);
// same as nk_tt_GetCodepointBitmapBox, but you can specify a subpixel
// shift for the character

// the following functions are equivalent to the above functions, but operate
// on glyph indices instead of Unicode codepoints (for efficiency)
NK_TT_DEF unsigned char *nk_tt_GetGlyphBitmap(const nk_tt_fontinfo *info, float scale_x, float scale_y, int glyph, int *width, int *height, int *xoff, int *yoff);
NK_TT_DEF unsigned char *nk_tt_GetGlyphBitmapSubpixel(const nk_tt_fontinfo *info, float scale_x, float scale_y, float shift_x, float shift_y, int glyph, int *width, int *height, int *xoff, int *yoff);
NK_TT_DEF void nk_tt_MakeGlyphBitmap(const nk_tt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int glyph);
NK_TT_DEF void nk_tt_MakeGlyphBitmapSubpixel(const nk_tt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph);
NK_TT_DEF void nk_tt_MakeGlyphBitmapSubpixelPrefilter(const nk_tt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int oversample_x, int oversample_y, float *sub_x, float *sub_y, int glyph);
NK_TT_DEF void nk_tt_GetGlyphBitmapBox(const nk_tt_fontinfo *font, int glyph, float scale_x, float scale_y, int *ix0, int *iy0, int *ix1, int *iy1);
NK_TT_DEF void nk_tt_GetGlyphBitmapBoxSubpixel(const nk_tt_fontinfo *font, int glyph, float scale_x, float scale_y,float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1);


// @TODO: don't expose this structure
typedef struct
{
   int w,h,stride;
   unsigned char *pixels;
} nk_tt__bitmap;

// rasterize a shape with quadratic beziers into a bitmap
NK_TT_DEF void nk_tt_Rasterize(nk_tt__bitmap *result,        // 1-channel bitmap to draw into
                               float flatness_in_pixels,     // allowable error of curve in pixels
                               nk_tt_vertex *vertices,       // array of vertices defining shape
                               int num_verts,                // number of vertices in above array
                               float scale_x, float scale_y, // scale applied to input vertices
                               float shift_x, float shift_y, // translation applied to input vertices
                               int x_off, int y_off,         // another translation applied to input
                               int invert,                   // if non-zero, vertically flip shape
                               void *userdata);              // context for to NK_TT_MALLOC

//////////////////////////////////////////////////////////////////////////////
//
// Signed Distance Function (or Field) rendering

NK_TT_DEF void nk_tt_FreeSDF(unsigned char *bitmap, void *userdata);
// frees the SDF bitmap allocated below

NK_TT_DEF unsigned char * nk_tt_GetGlyphSDF(const nk_tt_fontinfo *info, float scale, int glyph, int padding, unsigned char onedge_value, float pixel_dist_scale, int *width, int *height, int *xoff, int *yoff);
NK_TT_DEF unsigned char * nk_tt_GetCodepointSDF(const nk_tt_fontinfo *info, float scale, int codepoint, int padding, unsigned char onedge_value, float pixel_dist_scale, int *width, int *height, int *xoff, int *yoff);
// These functions compute a discretized SDF field for a single character, suitable for storing
// in a single-channel texture, sampling with bilinear filtering, and testing against
// larger than some threshold to produce scalable fonts.
//        info              --  the font
//        scale             --  controls the size of the resulting SDF bitmap, same as it would be creating a regular bitmap
//        glyph/codepoint   --  the character to generate the SDF for
//        padding           --  extra "pixels" around the character which are filled with the distance to the character (not 0),
//                                 which allows effects like bit outlines
//        onedge_value      --  value 0-255 to test the SDF against to reconstruct the character (i.e. the isocontour of the character)
//        pixel_dist_scale  --  what value the SDF should increase by when moving one SDF "pixel" away from the edge (on the 0..255 scale)
//                                 if positive, > onedge_value is inside; if negative, < onedge_value is inside
//        width,height      --  output height & width of the SDF bitmap (including padding)
//        xoff,yoff         --  output origin of the character
//        return value      --  a 2D array of bytes 0..255, width*height in size
//
// pixel_dist_scale & onedge_value are a scale & bias that allows you to make
// optimal use of the limited 0..255 for your application, trading off precision
// and special effects. SDF values outside the range 0..255 are clamped to 0..255.
//
// Example:
//      scale = nk_tt_ScaleForPixelHeight(22)
//      padding = 5
//      onedge_value = 180
//      pixel_dist_scale = 180/5.0 = 36.0
//
//      This will create an SDF bitmap in which the character is about 22 pixels
//      high but the whole bitmap is about 22+5+5=32 pixels high. To produce a filled
//      shape, sample the SDF at each pixel and fill the pixel if the SDF value
//      is greater than or equal to 180/255. (You'll actually want to antialias,
//      which is beyond the scope of this example.) Additionally, you can compute
//      offset outlines (e.g. to stroke the character border inside & outside,
//      or only outside). For example, to fill outside the character up to 3 SDF
//      pixels, you would compare against (180-36.0*3)/255 = 72/255. The above
//      choice of variables maps a range from 5 pixels outside the shape to
//      2 pixels inside the shape to 0..255; this is intended primarily for apply
//      outside effects only (the interior range is needed to allow proper
//      antialiasing of the font at *smaller* sizes)
//
// The function computes the SDF analytically at each SDF pixel, not by e.g.
// building a higher-res bitmap and approximating it. In theory the quality
// should be as high as possible for an SDF of this size & representation, but
// unclear if this is true in practice (perhaps building a higher-res bitmap
// and computing from that can allow drop-out prevention).
//
// The algorithm has not been optimized at all, so expect it to be slow
// if computing lots of characters or very large sizes. 



//////////////////////////////////////////////////////////////////////////////
//
// Finding the right font...
//
// You should really just solve this offline, keep your own tables
// of what font is what, and don't try to get it out of the .ttf file.
// That's because getting it out of the .ttf file is really hard, because
// the names in the file can appear in many possible encodings, in many
// possible languages, and e.g. if you need a case-insensitive comparison,
// the details of that depend on the encoding & language in a complex way
// (actually underspecified in truetype, but also gigantic).
//
// But you can use the provided functions in two possible ways:
//     nk_tt_FindMatchingFont() will use *case-sensitive* comparisons on
//             unicode-encoded names to try to find the font you want;
//             you can run this before calling nk_tt_InitFont()
//
//     nk_tt_GetFontNameString() lets you get any of the various strings
//             from the file yourself and do your own comparisons on them.
//             You have to have called nk_tt_InitFont() first.


NK_TT_DEF int nk_tt_FindMatchingFont(const unsigned char *fontdata, const char *name, int flags);
// returns the offset (not index) of the font that matches, or -1 if none
//   if you use NK_TT_MACSTYLE_DONTCARE, use a font name like "Arial Bold".
//   if you use any other flag, use a font name like "Arial"; this checks
//     the 'macStyle' header field; i don't know if fonts set this consistently
#define NK_TT_MACSTYLE_DONTCARE     0
#define NK_TT_MACSTYLE_BOLD         1
#define NK_TT_MACSTYLE_ITALIC       2
#define NK_TT_MACSTYLE_UNDERSCORE   4
#define NK_TT_MACSTYLE_NONE         8   // <= not same as 0, this makes us check the bitfield is 0

NK_TT_DEF int nk_tt_CompareUTF8toUTF16_bigendian(const char *s1, int len1, const char *s2, int len2);
// returns 1/0 whether the first string interpreted as utf8 is identical to
// the second string interpreted as big-endian utf16... useful for strings from next func

NK_TT_DEF const char *nk_tt_GetFontNameString(const nk_tt_fontinfo *font, int *length, int platformID, int encodingID, int languageID, int nameID);
// returns the string (which may be big-endian double byte, e.g. for unicode)
// and puts the length in bytes in *length.
//
// some of the values for the IDs are below; for more see the truetype spec:
//     http://developer.apple.com/textfonts/TTRefMan/RM06/Chap6name.html
//     http://www.microsoft.com/typography/otspec/name.htm

enum { // platformID
   NK_TT_PLATFORM_ID_UNICODE   =0,
   NK_TT_PLATFORM_ID_MAC       =1,
   NK_TT_PLATFORM_ID_ISO       =2,
   NK_TT_PLATFORM_ID_MICROSOFT =3
};

enum { // encodingID for NK_TT_PLATFORM_ID_UNICODE
   NK_TT_UNICODE_EID_UNICODE_1_0    =0,
   NK_TT_UNICODE_EID_UNICODE_1_1    =1,
   NK_TT_UNICODE_EID_ISO_10646      =2,
   NK_TT_UNICODE_EID_UNICODE_2_0_BMP=3,
   NK_TT_UNICODE_EID_UNICODE_2_0_FULL=4
};

enum { // encodingID for NK_TT_PLATFORM_ID_MICROSOFT
   NK_TT_MS_EID_SYMBOL        =0,
   NK_TT_MS_EID_UNICODE_BMP   =1,
   NK_TT_MS_EID_SHIFTJIS      =2,
   NK_TT_MS_EID_UNICODE_FULL  =10
};

enum { // encodingID for NK_TT_PLATFORM_ID_MAC; same as Script Manager codes
   NK_TT_MAC_EID_ROMAN        =0,   NK_TT_MAC_EID_ARABIC       =4,
   NK_TT_MAC_EID_JAPANESE     =1,   NK_TT_MAC_EID_HEBREW       =5,
   NK_TT_MAC_EID_CHINESE_TRAD =2,   NK_TT_MAC_EID_GREEK        =6,
   NK_TT_MAC_EID_KOREAN       =3,   NK_TT_MAC_EID_RUSSIAN      =7
};

enum { // languageID for NK_TT_PLATFORM_ID_MICROSOFT; same as LCID...
       // problematic because there are e.g. 16 english LCIDs and 16 arabic LCIDs
   NK_TT_MS_LANG_ENGLISH     =0x0409,   NK_TT_MS_LANG_ITALIAN     =0x0410,
   NK_TT_MS_LANG_CHINESE     =0x0804,   NK_TT_MS_LANG_JAPANESE    =0x0411,
   NK_TT_MS_LANG_DUTCH       =0x0413,   NK_TT_MS_LANG_KOREAN      =0x0412,
   NK_TT_MS_LANG_FRENCH      =0x040c,   NK_TT_MS_LANG_RUSSIAN     =0x0419,
   NK_TT_MS_LANG_GERMAN      =0x0407,   NK_TT_MS_LANG_SPANISH     =0x0409,
   NK_TT_MS_LANG_HEBREW      =0x040d,   NK_TT_MS_LANG_SWEDISH     =0x041D
};

enum { // languageID for NK_TT_PLATFORM_ID_MAC
   NK_TT_MAC_LANG_ENGLISH      =0 ,   NK_TT_MAC_LANG_JAPANESE     =11,
   NK_TT_MAC_LANG_ARABIC       =12,   NK_TT_MAC_LANG_KOREAN       =23,
   NK_TT_MAC_LANG_DUTCH        =4 ,   NK_TT_MAC_LANG_RUSSIAN      =32,
   NK_TT_MAC_LANG_FRENCH       =1 ,   NK_TT_MAC_LANG_SPANISH      =6 ,
   NK_TT_MAC_LANG_GERMAN       =2 ,   NK_TT_MAC_LANG_SWEDISH      =5 ,
   NK_TT_MAC_LANG_HEBREW       =10,   NK_TT_MAC_LANG_CHINESE_SIMPLIFIED =33,
   NK_TT_MAC_LANG_ITALIAN      =3 ,   NK_TT_MAC_LANG_CHINESE_TRAD =19
};

#ifdef __cplusplus
}
#endif

#endif // __STB_INCLUDE_STB_TRUETYPE_H__

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////   IMPLEMENTATION
////
////

#if 1 // def STB_TRUETYPE_IMPLEMENTATION

#ifndef NK_TT_MAX_OVERSAMPLE
#define NK_TT_MAX_OVERSAMPLE   8
#endif

#if NK_TT_MAX_OVERSAMPLE > 255
#error "NK_TT_MAX_OVERSAMPLE cannot be > 255"
#endif

typedef int nk_tt__test_oversample_pow2[(NK_TT_MAX_OVERSAMPLE & (NK_TT_MAX_OVERSAMPLE-1)) == 0 ? 1 : -1];

#ifndef NK_TT_RASTERIZER_VERSION
#define NK_TT_RASTERIZER_VERSION 2
#endif

#ifdef _MSC_VER
#define NK_TT__NOTUSED(v)  (void)(v)
#else
#define NK_TT__NOTUSED(v)  (void)sizeof(v)
#endif

//////////////////////////////////////////////////////////////////////////
//
// nk_tt__buf helpers to parse data from file
//

static nk_tt_uint8 nk_tt__buf_get8(nk_tt__buf *b)
{
   if (b->cursor >= b->size)
      return 0;
   return b->data[b->cursor++];
}

static nk_tt_uint8 nk_tt__buf_peek8(nk_tt__buf *b)
{
   if (b->cursor >= b->size)
      return 0;
   return b->data[b->cursor];
}

static void nk_tt__buf_seek(nk_tt__buf *b, int o)
{
   NK_TT_assert(!(o > b->size || o < 0));
   b->cursor = (o > b->size || o < 0) ? b->size : o;
}

static void nk_tt__buf_skip(nk_tt__buf *b, int o)
{
   nk_tt__buf_seek(b, b->cursor + o);
}

static nk_tt_uint32 nk_tt__buf_get(nk_tt__buf *b, int n)
{
   nk_tt_uint32 v = 0;
   int i;
   NK_TT_assert(n >= 1 && n <= 4);
   for (i = 0; i < n; i++)
      v = (v << 8) | nk_tt__buf_get8(b);
   return v;
}

static nk_tt__buf nk_tt__new_buf(const void *p, size_t size)
{
   nk_tt__buf r;
   NK_TT_assert(size < 0x40000000);
   r.data = (nk_tt_uint8*) p;
   r.size = (int) size;
   r.cursor = 0;
   return r;
}

#define nk_tt__buf_get16(b)  nk_tt__buf_get((b), 2)
#define nk_tt__buf_get32(b)  nk_tt__buf_get((b), 4)

static nk_tt__buf nk_tt__buf_range(const nk_tt__buf *b, int o, int s)
{
   nk_tt__buf r = nk_tt__new_buf(NULL, 0);
   if (o < 0 || s < 0 || o > b->size || s > b->size - o) return r;
   r.data = b->data + o;
   r.size = s;
   return r;
}

static nk_tt__buf nk_tt__cff_get_index(nk_tt__buf *b)
{
   int count, start, offsize;
   start = b->cursor;
   count = nk_tt__buf_get16(b);
   if (count) {
      offsize = nk_tt__buf_get8(b);
      NK_TT_assert(offsize >= 1 && offsize <= 4);
      nk_tt__buf_skip(b, offsize * count);
      nk_tt__buf_skip(b, nk_tt__buf_get(b, offsize) - 1);
   }
   return nk_tt__buf_range(b, start, b->cursor - start);
}

static nk_tt_uint32 nk_tt__cff_int(nk_tt__buf *b)
{
   int b0 = nk_tt__buf_get8(b);
   if (b0 >= 32 && b0 <= 246)       return b0 - 139;
   else if (b0 >= 247 && b0 <= 250) return (b0 - 247)*256 + nk_tt__buf_get8(b) + 108;
   else if (b0 >= 251 && b0 <= 254) return -(b0 - 251)*256 - nk_tt__buf_get8(b) - 108;
   else if (b0 == 28)               return nk_tt__buf_get16(b);
   else if (b0 == 29)               return nk_tt__buf_get32(b);
   NK_TT_assert(0);
   return 0;
}

static void nk_tt__cff_skip_operand(nk_tt__buf *b) {
   int v, b0 = nk_tt__buf_peek8(b);
   NK_TT_assert(b0 >= 28);
   if (b0 == 30) {
      nk_tt__buf_skip(b, 1);
      while (b->cursor < b->size) {
         v = nk_tt__buf_get8(b);
         if ((v & 0xF) == 0xF || (v >> 4) == 0xF)
            break;
      }
   } else {
      nk_tt__cff_int(b);
   }
}

static nk_tt__buf nk_tt__dict_get(nk_tt__buf *b, int key)
{
   nk_tt__buf_seek(b, 0);
   while (b->cursor < b->size) {
      int start = b->cursor, end, op;
      while (nk_tt__buf_peek8(b) >= 28)
         nk_tt__cff_skip_operand(b);
      end = b->cursor;
      op = nk_tt__buf_get8(b);
      if (op == 12)  op = nk_tt__buf_get8(b) | 0x100;
      if (op == key) return nk_tt__buf_range(b, start, end-start);
   }
   return nk_tt__buf_range(b, 0, 0);
}

static void nk_tt__dict_get_ints(nk_tt__buf *b, int key, int outcount, nk_tt_uint32 *out)
{
   int i;
   nk_tt__buf operands = nk_tt__dict_get(b, key);
   for (i = 0; i < outcount && operands.cursor < operands.size; i++)
      out[i] = nk_tt__cff_int(&operands);
}

static int nk_tt__cff_index_count(nk_tt__buf *b)
{
   nk_tt__buf_seek(b, 0);
   return nk_tt__buf_get16(b);
}

static nk_tt__buf nk_tt__cff_index_get(nk_tt__buf b, int i)
{
   int count, offsize, start, end;
   nk_tt__buf_seek(&b, 0);
   count = nk_tt__buf_get16(&b);
   offsize = nk_tt__buf_get8(&b);
   NK_TT_assert(i >= 0 && i < count);
   NK_TT_assert(offsize >= 1 && offsize <= 4);
   nk_tt__buf_skip(&b, i*offsize);
   start = nk_tt__buf_get(&b, offsize);
   end = nk_tt__buf_get(&b, offsize);
   return nk_tt__buf_range(&b, 2+(count+1)*offsize+start, end - start);
}

//////////////////////////////////////////////////////////////////////////
//
// accessors to parse data from file
//

// on platforms that don't allow misaligned reads, if we want to allow
// truetype fonts that aren't padded to alignment, define ALLOW_UNALIGNED_TRUETYPE

#define ttBYTE(p)     (* (nk_tt_uint8 *) (p))
#define ttCHAR(p)     (* (nk_tt_int8 *) (p))
#define ttFixed(p)    ttLONG(p)

static nk_tt_uint16 ttUSHORT(nk_tt_uint8 *p) { return p[0]*256 + p[1]; }
static nk_tt_int16 ttSHORT(nk_tt_uint8 *p)   { return p[0]*256 + p[1]; }
static nk_tt_uint32 ttULONG(nk_tt_uint8 *p)  { return (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3]; }
static nk_tt_int32 ttLONG(nk_tt_uint8 *p)    { return (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3]; }

#define nk_tt_tag4(p,c0,c1,c2,c3) ((p)[0] == (c0) && (p)[1] == (c1) && (p)[2] == (c2) && (p)[3] == (c3))
#define nk_tt_tag(p,str)           nk_tt_tag4(p,str[0],str[1],str[2],str[3])

static int nk_tt__isfont(nk_tt_uint8 *font)
{
   // check the version number
   if (nk_tt_tag4(font, '1',0,0,0))  return 1; // TrueType 1
   if (nk_tt_tag(font, "typ1"))   return 1; // TrueType with type 1 font -- we don't support this!
   if (nk_tt_tag(font, "OTTO"))   return 1; // OpenType with CFF
   if (nk_tt_tag4(font, 0,1,0,0)) return 1; // OpenType 1.0
   if (nk_tt_tag(font, "true"))   return 1; // Apple specification for TrueType fonts
   return 0;
}

// @OPTIMIZE: binary search
static nk_tt_uint32 nk_tt__find_table(nk_tt_uint8 *data, nk_tt_uint32 fontstart, const char *tag)
{
   nk_tt_int32 num_tables = ttUSHORT(data+fontstart+4);
   nk_tt_uint32 tabledir = fontstart + 12;
   nk_tt_int32 i;
   for (i=0; i < num_tables; ++i) {
      nk_tt_uint32 loc = tabledir + 16*i;
      if (nk_tt_tag(data+loc+0, tag))
         return ttULONG(data+loc+8);
   }
   return 0;
}

static int nk_tt_GetFontOffsetForIndex_internal(unsigned char *font_collection, int index)
{
   // if it's just a font, there's only one valid index
   if (nk_tt__isfont(font_collection))
      return index == 0 ? 0 : -1;

   // check if it's a TTC
   if (nk_tt_tag(font_collection, "ttcf")) {
      // version 1?
      if (ttULONG(font_collection+4) == 0x00010000 || ttULONG(font_collection+4) == 0x00020000) {
         nk_tt_int32 n = ttLONG(font_collection+8);
         if (index >= n)
            return -1;
         return ttULONG(font_collection+12+index*4);
      }
   }
   return -1;
}

static int nk_tt_GetNumberOfFonts_internal(unsigned char *font_collection)
{
   // if it's just a font, there's only one valid font
   if (nk_tt__isfont(font_collection))
      return 1;

   // check if it's a TTC
   if (nk_tt_tag(font_collection, "ttcf")) {
      // version 1?
      if (ttULONG(font_collection+4) == 0x00010000 || ttULONG(font_collection+4) == 0x00020000) {
         return ttLONG(font_collection+8);
      }
   }
   return 0;
}

static nk_tt__buf nk_tt__get_subrs(nk_tt__buf cff, nk_tt__buf fontdict)
{
   nk_tt_uint32 subrsoff = 0, private_loc[2] = { 0, 0 };
   nk_tt__buf pdict;
   nk_tt__dict_get_ints(&fontdict, 18, 2, private_loc);
   if (!private_loc[1] || !private_loc[0]) return nk_tt__new_buf(NULL, 0);
   pdict = nk_tt__buf_range(&cff, private_loc[1], private_loc[0]);
   nk_tt__dict_get_ints(&pdict, 19, 1, &subrsoff);
   if (!subrsoff) return nk_tt__new_buf(NULL, 0);
   nk_tt__buf_seek(&cff, private_loc[1]+subrsoff);
   return nk_tt__cff_get_index(&cff);
}

static int nk_tt_InitFont_internal(nk_tt_fontinfo *info, unsigned char *data, int fontstart)
{
   nk_tt_uint32 cmap, t;
   nk_tt_int32 i,numTables;

   info->data = data;
   info->fontstart = fontstart;
   info->cff = nk_tt__new_buf(NULL, 0);

   cmap = nk_tt__find_table(data, fontstart, "cmap");       // required
   info->loca = nk_tt__find_table(data, fontstart, "loca"); // required
   info->head = nk_tt__find_table(data, fontstart, "head"); // required
   info->glyf = nk_tt__find_table(data, fontstart, "glyf"); // required
   info->hhea = nk_tt__find_table(data, fontstart, "hhea"); // required
   info->hmtx = nk_tt__find_table(data, fontstart, "hmtx"); // required
   info->kern = nk_tt__find_table(data, fontstart, "kern"); // not required
   info->gpos = nk_tt__find_table(data, fontstart, "GPOS"); // not required

   if (!cmap || !info->head || !info->hhea || !info->hmtx)
      return 0;
   if (info->glyf) {
      // required for truetype
      if (!info->loca) return 0;
   } else {
      // initialization for CFF / Type2 fonts (OTF)
      nk_tt__buf b, topdict, topdictidx;
      nk_tt_uint32 cstype = 2, charstrings = 0, fdarrayoff = 0, fdselectoff = 0;
      nk_tt_uint32 cff;

      cff = nk_tt__find_table(data, fontstart, "CFF ");
      if (!cff) return 0;

      info->fontdicts = nk_tt__new_buf(NULL, 0);
      info->fdselect = nk_tt__new_buf(NULL, 0);

      // @TODO this should use size from table (not 512MB)
      info->cff = nk_tt__new_buf(data+cff, 512*1024*1024);
      b = info->cff;

      // read the header
      nk_tt__buf_skip(&b, 2);
      nk_tt__buf_seek(&b, nk_tt__buf_get8(&b)); // hdrsize

      // @TODO the name INDEX could list multiple fonts,
      // but we just use the first one.
      nk_tt__cff_get_index(&b);  // name INDEX
      topdictidx = nk_tt__cff_get_index(&b);
      topdict = nk_tt__cff_index_get(topdictidx, 0);
      nk_tt__cff_get_index(&b);  // string INDEX
      info->gsubrs = nk_tt__cff_get_index(&b);

      nk_tt__dict_get_ints(&topdict, 17, 1, &charstrings);
      nk_tt__dict_get_ints(&topdict, 0x100 | 6, 1, &cstype);
      nk_tt__dict_get_ints(&topdict, 0x100 | 36, 1, &fdarrayoff);
      nk_tt__dict_get_ints(&topdict, 0x100 | 37, 1, &fdselectoff);
      info->subrs = nk_tt__get_subrs(b, topdict);

      // we only support Type 2 charstrings
      if (cstype != 2) return 0;
      if (charstrings == 0) return 0;

      if (fdarrayoff) {
         // looks like a CID font
         if (!fdselectoff) return 0;
         nk_tt__buf_seek(&b, fdarrayoff);
         info->fontdicts = nk_tt__cff_get_index(&b);
         info->fdselect = nk_tt__buf_range(&b, fdselectoff, b.size-fdselectoff);
      }

      nk_tt__buf_seek(&b, charstrings);
      info->charstrings = nk_tt__cff_get_index(&b);
   }

   t = nk_tt__find_table(data, fontstart, "maxp");
   if (t)
      info->numGlyphs = ttUSHORT(data+t+4);
   else
      info->numGlyphs = 0xffff;

   // find a cmap encoding table we understand *now* to avoid searching
   // later. (todo: could make this installable)
   // the same regardless of glyph.
   numTables = ttUSHORT(data + cmap + 2);
   info->index_map = 0;
   for (i=0; i < numTables; ++i) {
      nk_tt_uint32 encoding_record = cmap + 4 + 8 * i;
      // find an encoding we understand:
      switch(ttUSHORT(data+encoding_record)) {
         case NK_TT_PLATFORM_ID_MICROSOFT:
            switch (ttUSHORT(data+encoding_record+2)) {
               case NK_TT_MS_EID_UNICODE_BMP:
               case NK_TT_MS_EID_UNICODE_FULL:
                  // MS/Unicode
                  info->index_map = cmap + ttULONG(data+encoding_record+4);
                  break;
            }
            break;
        case NK_TT_PLATFORM_ID_UNICODE:
            // Mac/iOS has these
            // all the encodingIDs are unicode, so we don't bother to check it
            info->index_map = cmap + ttULONG(data+encoding_record+4);
            break;
      }
   }
   if (info->index_map == 0)
      return 0;

   info->indexToLocFormat = ttUSHORT(data+info->head + 50);
   return 1;
}

NK_TT_DEF int nk_tt_FindGlyphIndex(const nk_tt_fontinfo *info, int unicode_codepoint)
{
   nk_tt_uint8 *data = info->data;
   nk_tt_uint32 index_map = info->index_map;

   nk_tt_uint16 format = ttUSHORT(data + index_map + 0);
   if (format == 0) { // apple byte encoding
      nk_tt_int32 bytes = ttUSHORT(data + index_map + 2);
      if (unicode_codepoint < bytes-6)
         return ttBYTE(data + index_map + 6 + unicode_codepoint);
      return 0;
   } else if (format == 6) {
      nk_tt_uint32 first = ttUSHORT(data + index_map + 6);
      nk_tt_uint32 count = ttUSHORT(data + index_map + 8);
      if ((nk_tt_uint32) unicode_codepoint >= first && (nk_tt_uint32) unicode_codepoint < first+count)
         return ttUSHORT(data + index_map + 10 + (unicode_codepoint - first)*2);
      return 0;
   } else if (format == 2) {
      NK_TT_assert(0); // @TODO: high-byte mapping for japanese/chinese/korean
      return 0;
   } else if (format == 4) { // standard mapping for windows fonts: binary search collection of ranges
      nk_tt_uint16 segcount = ttUSHORT(data+index_map+6) >> 1;
      nk_tt_uint16 searchRange = ttUSHORT(data+index_map+8) >> 1;
      nk_tt_uint16 entrySelector = ttUSHORT(data+index_map+10);
      nk_tt_uint16 rangeShift = ttUSHORT(data+index_map+12) >> 1;

      // do a binary search of the segments
      nk_tt_uint32 endCount = index_map + 14;
      nk_tt_uint32 search = endCount;

      if (unicode_codepoint > 0xffff)
         return 0;

      // they lie from endCount .. endCount + segCount
      // but searchRange is the nearest power of two, so...
      if (unicode_codepoint >= ttUSHORT(data + search + rangeShift*2))
         search += rangeShift*2;

      // now decrement to bias correctly to find smallest
      search -= 2;
      while (entrySelector) {
         nk_tt_uint16 end;
         searchRange >>= 1;
         end = ttUSHORT(data + search + searchRange*2);
         if (unicode_codepoint > end)
            search += searchRange*2;
         --entrySelector;
      }
      search += 2;

      {
         nk_tt_uint16 offset, start;
         nk_tt_uint16 item = (nk_tt_uint16) ((search - endCount) >> 1);

         NK_TT_assert(unicode_codepoint <= ttUSHORT(data + endCount + 2*item));
         start = ttUSHORT(data + index_map + 14 + segcount*2 + 2 + 2*item);
         if (unicode_codepoint < start)
            return 0;

         offset = ttUSHORT(data + index_map + 14 + segcount*6 + 2 + 2*item);
         if (offset == 0)
            return (nk_tt_uint16) (unicode_codepoint + ttSHORT(data + index_map + 14 + segcount*4 + 2 + 2*item));

         return ttUSHORT(data + offset + (unicode_codepoint-start)*2 + index_map + 14 + segcount*6 + 2 + 2*item);
      }
   } else if (format == 12 || format == 13) {
      nk_tt_uint32 ngroups = ttULONG(data+index_map+12);
      nk_tt_int32 low,high;
      low = 0; high = (nk_tt_int32)ngroups;
      // Binary search the right group.
      while (low < high) {
         nk_tt_int32 mid = low + ((high-low) >> 1); // rounds down, so low <= mid < high
         nk_tt_uint32 start_char = ttULONG(data+index_map+16+mid*12);
         nk_tt_uint32 end_char = ttULONG(data+index_map+16+mid*12+4);
         if ((nk_tt_uint32) unicode_codepoint < start_char)
            high = mid;
         else if ((nk_tt_uint32) unicode_codepoint > end_char)
            low = mid+1;
         else {
            nk_tt_uint32 start_glyph = ttULONG(data+index_map+16+mid*12+8);
            if (format == 12)
               return start_glyph + unicode_codepoint-start_char;
            else // format == 13
               return start_glyph;
         }
      }
      return 0; // not found
   }
   // @TODO
   NK_TT_assert(0);
   return 0;
}

NK_TT_DEF int nk_tt_GetCodepointShape(const nk_tt_fontinfo *info, int unicode_codepoint, nk_tt_vertex **vertices)
{
   return nk_tt_GetGlyphShape(info, nk_tt_FindGlyphIndex(info, unicode_codepoint), vertices);
}

static void nk_tt_setvertex(nk_tt_vertex *v, nk_tt_uint8 type, nk_tt_int32 x, nk_tt_int32 y, nk_tt_int32 cx, nk_tt_int32 cy)
{
   v->type = type;
   v->x = (nk_tt_int16) x;
   v->y = (nk_tt_int16) y;
   v->cx = (nk_tt_int16) cx;
   v->cy = (nk_tt_int16) cy;
}

static int nk_tt__GetGlyfOffset(const nk_tt_fontinfo *info, int glyph_index)
{
   int g1,g2;

   NK_TT_assert(!info->cff.size);

   if (glyph_index >= info->numGlyphs) return -1; // glyph index out of range
   if (info->indexToLocFormat >= 2)    return -1; // unknown index->glyph map format

   if (info->indexToLocFormat == 0) {
      g1 = info->glyf + ttUSHORT(info->data + info->loca + glyph_index * 2) * 2;
      g2 = info->glyf + ttUSHORT(info->data + info->loca + glyph_index * 2 + 2) * 2;
   } else {
      g1 = info->glyf + ttULONG (info->data + info->loca + glyph_index * 4);
      g2 = info->glyf + ttULONG (info->data + info->loca + glyph_index * 4 + 4);
   }

   return g1==g2 ? -1 : g1; // if length is 0, return -1
}

static int nk_tt__GetGlyphInfoT2(const nk_tt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1);

NK_TT_DEF int nk_tt_GetGlyphBox(const nk_tt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1)
{
   if (info->cff.size) {
      nk_tt__GetGlyphInfoT2(info, glyph_index, x0, y0, x1, y1);
   } else {
      int g = nk_tt__GetGlyfOffset(info, glyph_index);
      if (g < 0) return 0;

      if (x0) *x0 = ttSHORT(info->data + g + 2);
      if (y0) *y0 = ttSHORT(info->data + g + 4);
      if (x1) *x1 = ttSHORT(info->data + g + 6);
      if (y1) *y1 = ttSHORT(info->data + g + 8);
   }
   return 1;
}

NK_TT_DEF int nk_tt_GetCodepointBox(const nk_tt_fontinfo *info, int codepoint, int *x0, int *y0, int *x1, int *y1)
{
   return nk_tt_GetGlyphBox(info, nk_tt_FindGlyphIndex(info,codepoint), x0,y0,x1,y1);
}

NK_TT_DEF int nk_tt_IsGlyphEmpty(const nk_tt_fontinfo *info, int glyph_index)
{
   nk_tt_int16 numberOfContours;
   int g;
   if (info->cff.size)
      return nk_tt__GetGlyphInfoT2(info, glyph_index, NULL, NULL, NULL, NULL) == 0;
   g = nk_tt__GetGlyfOffset(info, glyph_index);
   if (g < 0) return 1;
   numberOfContours = ttSHORT(info->data + g);
   return numberOfContours == 0;
}

static int nk_tt__close_shape(nk_tt_vertex *vertices, int num_vertices, int was_off, int start_off,
    nk_tt_int32 sx, nk_tt_int32 sy, nk_tt_int32 scx, nk_tt_int32 scy, nk_tt_int32 cx, nk_tt_int32 cy)
{
   if (start_off) {
      if (was_off)
         nk_tt_setvertex(&vertices[num_vertices++], NK_TT_vcurve, (cx+scx)>>1, (cy+scy)>>1, cx,cy);
      nk_tt_setvertex(&vertices[num_vertices++], NK_TT_vcurve, sx,sy,scx,scy);
   } else {
      if (was_off)
         nk_tt_setvertex(&vertices[num_vertices++], NK_TT_vcurve,sx,sy,cx,cy);
      else
         nk_tt_setvertex(&vertices[num_vertices++], NK_TT_vline,sx,sy,0,0);
   }
   return num_vertices;
}

static int nk_tt__GetGlyphShapeTT(const nk_tt_fontinfo *info, int glyph_index, nk_tt_vertex **pvertices)
{
   nk_tt_int16 numberOfContours;
   nk_tt_uint8 *endPtsOfContours;
   nk_tt_uint8 *data = info->data;
   nk_tt_vertex *vertices=0;
   int num_vertices=0;
   int g = nk_tt__GetGlyfOffset(info, glyph_index);

   *pvertices = NULL;

   if (g < 0) return 0;

   numberOfContours = ttSHORT(data + g);

   if (numberOfContours > 0) {
      nk_tt_uint8 flags=0,flagcount;
      nk_tt_int32 ins, i,j=0,m,n, next_move, was_off=0, off, start_off=0;
      nk_tt_int32 x,y,cx,cy,sx,sy, scx,scy;
      nk_tt_uint8 *points;
      endPtsOfContours = (data + g + 10);
      ins = ttUSHORT(data + g + 10 + numberOfContours * 2);
      points = data + g + 10 + numberOfContours * 2 + 2 + ins;

      n = 1+ttUSHORT(endPtsOfContours + numberOfContours*2-2);

      m = n + 2*numberOfContours;  // a loose bound on how many vertices we might need
      vertices = (nk_tt_vertex *) NK_TT_malloc(m * sizeof(vertices[0]), info->userdata);
      if (vertices == 0)
         return 0;

      next_move = 0;
      flagcount=0;

      // in first pass, we load uninterpreted data into the allocated array
      // above, shifted to the end of the array so we won't overwrite it when
      // we create our final data starting from the front

      off = m - n; // starting offset for uninterpreted data, regardless of how m ends up being calculated

      // first load flags

      for (i=0; i < n; ++i) {
         if (flagcount == 0) {
            flags = *points++;
            if (flags & 8)
               flagcount = *points++;
         } else
            --flagcount;
         vertices[off+i].type = flags;
      }

      // now load x coordinates
      x=0;
      for (i=0; i < n; ++i) {
         flags = vertices[off+i].type;
         if (flags & 2) {
            nk_tt_int16 dx = *points++;
            x += (flags & 16) ? dx : -dx; // ???
         } else {
            if (!(flags & 16)) {
               x = x + (nk_tt_int16) (points[0]*256 + points[1]);
               points += 2;
            }
         }
         vertices[off+i].x = (nk_tt_int16) x;
      }

      // now load y coordinates
      y=0;
      for (i=0; i < n; ++i) {
         flags = vertices[off+i].type;
         if (flags & 4) {
            nk_tt_int16 dy = *points++;
            y += (flags & 32) ? dy : -dy; // ???
         } else {
            if (!(flags & 32)) {
               y = y + (nk_tt_int16) (points[0]*256 + points[1]);
               points += 2;
            }
         }
         vertices[off+i].y = (nk_tt_int16) y;
      }

      // now convert them to our format
      num_vertices=0;
      sx = sy = cx = cy = scx = scy = 0;
      for (i=0; i < n; ++i) {
         flags = vertices[off+i].type;
         x     = (nk_tt_int16) vertices[off+i].x;
         y     = (nk_tt_int16) vertices[off+i].y;

         if (next_move == i) {
            if (i != 0)
               num_vertices = nk_tt__close_shape(vertices, num_vertices, was_off, start_off, sx,sy,scx,scy,cx,cy);

            // now start the new one               
            start_off = !(flags & 1);
            if (start_off) {
               // if we start off with an off-curve point, then when we need to find a point on the curve
               // where we can start, and we need to save some state for when we wraparound.
               scx = x;
               scy = y;
               if (!(vertices[off+i+1].type & 1)) {
                  // next point is also a curve point, so interpolate an on-point curve
                  sx = (x + (nk_tt_int32) vertices[off+i+1].x) >> 1;
                  sy = (y + (nk_tt_int32) vertices[off+i+1].y) >> 1;
               } else {
                  // otherwise just use the next point as our start point
                  sx = (nk_tt_int32) vertices[off+i+1].x;
                  sy = (nk_tt_int32) vertices[off+i+1].y;
                  ++i; // we're using point i+1 as the starting point, so skip it
               }
            } else {
               sx = x;
               sy = y;
            }
            nk_tt_setvertex(&vertices[num_vertices++], NK_TT_vmove,sx,sy,0,0);
            was_off = 0;
            next_move = 1 + ttUSHORT(endPtsOfContours+j*2);
            ++j;
         } else {
            if (!(flags & 1)) { // if it's a curve
               if (was_off) // two off-curve control points in a row means interpolate an on-curve midpoint
                  nk_tt_setvertex(&vertices[num_vertices++], NK_TT_vcurve, (cx+x)>>1, (cy+y)>>1, cx, cy);
               cx = x;
               cy = y;
               was_off = 1;
            } else {
               if (was_off)
                  nk_tt_setvertex(&vertices[num_vertices++], NK_TT_vcurve, x,y, cx, cy);
               else
                  nk_tt_setvertex(&vertices[num_vertices++], NK_TT_vline, x,y,0,0);
               was_off = 0;
            }
         }
      }
      num_vertices = nk_tt__close_shape(vertices, num_vertices, was_off, start_off, sx,sy,scx,scy,cx,cy);
   } else if (numberOfContours == -1) {
      // Compound shapes.
      int more = 1;
      nk_tt_uint8 *comp = data + g + 10;
      num_vertices = 0;
      vertices = 0;
      while (more) {
         nk_tt_uint16 flags, gidx;
         int comp_num_verts = 0, i;
         nk_tt_vertex *comp_verts = 0, *tmp = 0;
         float mtx[6] = {1,0,0,1,0,0}, m, n;
         
         flags = ttSHORT(comp); comp+=2;
         gidx = ttSHORT(comp); comp+=2;

         if (flags & 2) { // XY values
            if (flags & 1) { // shorts
               mtx[4] = ttSHORT(comp); comp+=2;
               mtx[5] = ttSHORT(comp); comp+=2;
            } else {
               mtx[4] = ttCHAR(comp); comp+=1;
               mtx[5] = ttCHAR(comp); comp+=1;
            }
         }
         else {
            // @TODO handle matching point
            NK_TT_assert(0);
         }
         if (flags & (1<<3)) { // WE_HAVE_A_SCALE
            mtx[0] = mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[1] = mtx[2] = 0;
         } else if (flags & (1<<6)) { // WE_HAVE_AN_X_AND_YSCALE
            mtx[0] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[1] = mtx[2] = 0;
            mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
         } else if (flags & (1<<7)) { // WE_HAVE_A_TWO_BY_TWO
            mtx[0] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[1] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[2] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
         }
         
         // Find transformation scales.
         m = (float) NK_TT_sqrt(mtx[0]*mtx[0] + mtx[1]*mtx[1]);
         n = (float) NK_TT_sqrt(mtx[2]*mtx[2] + mtx[3]*mtx[3]);

         // Get indexed glyph.
         comp_num_verts = nk_tt_GetGlyphShape(info, gidx, &comp_verts);
         if (comp_num_verts > 0) {
            // Transform vertices.
            for (i = 0; i < comp_num_verts; ++i) {
               nk_tt_vertex* v = &comp_verts[i];
               nk_tt_vertex_type x,y;
               x=v->x; y=v->y;
               v->x = (nk_tt_vertex_type)(m * (mtx[0]*x + mtx[2]*y + mtx[4]));
               v->y = (nk_tt_vertex_type)(n * (mtx[1]*x + mtx[3]*y + mtx[5]));
               x=v->cx; y=v->cy;
               v->cx = (nk_tt_vertex_type)(m * (mtx[0]*x + mtx[2]*y + mtx[4]));
               v->cy = (nk_tt_vertex_type)(n * (mtx[1]*x + mtx[3]*y + mtx[5]));
            }
            // Append vertices.
            tmp = (nk_tt_vertex*)NK_TT_malloc((num_vertices+comp_num_verts)*sizeof(nk_tt_vertex), info->userdata);
            if (!tmp) {
               if (vertices) NK_TT_free(vertices, info->userdata);
               if (comp_verts) NK_TT_free(comp_verts, info->userdata);
               return 0;
            }
            if (num_vertices > 0) NK_TT_memcpy(tmp, vertices, num_vertices*sizeof(nk_tt_vertex));
            NK_TT_memcpy(tmp+num_vertices, comp_verts, comp_num_verts*sizeof(nk_tt_vertex));
            if (vertices) NK_TT_free(vertices, info->userdata);
            vertices = tmp;
            NK_TT_free(comp_verts, info->userdata);
            num_vertices += comp_num_verts;
         }
         // More components ?
         more = flags & (1<<5);
      }
   } else if (numberOfContours < 0) {
      // @TODO other compound variations?
      NK_TT_assert(0);
   } else {
      // numberOfCounters == 0, do nothing
   }

   *pvertices = vertices;
   return num_vertices;
}

typedef struct
{
   int bounds;
   int started;
   float first_x, first_y;
   float x, y;
   nk_tt_int32 min_x, max_x, min_y, max_y;

   nk_tt_vertex *pvertices;
   int num_vertices;
} nk_tt__csctx;

#define NK_TT__CSCTX_INIT(bounds) {bounds,0, 0,0, 0,0, 0,0,0,0, NULL, 0}

static void nk_tt__track_vertex(nk_tt__csctx *c, nk_tt_int32 x, nk_tt_int32 y)
{
   if (x > c->max_x || !c->started) c->max_x = x;
   if (y > c->max_y || !c->started) c->max_y = y;
   if (x < c->min_x || !c->started) c->min_x = x;
   if (y < c->min_y || !c->started) c->min_y = y;
   c->started = 1;
}

static void nk_tt__csctx_v(nk_tt__csctx *c, nk_tt_uint8 type, nk_tt_int32 x, nk_tt_int32 y, nk_tt_int32 cx, nk_tt_int32 cy, nk_tt_int32 cx1, nk_tt_int32 cy1)
{
   if (c->bounds) {
      nk_tt__track_vertex(c, x, y);
      if (type == NK_TT_vcubic) {
         nk_tt__track_vertex(c, cx, cy);
         nk_tt__track_vertex(c, cx1, cy1);
      }
   } else {
      nk_tt_setvertex(&c->pvertices[c->num_vertices], type, x, y, cx, cy);
      c->pvertices[c->num_vertices].cx1 = (nk_tt_int16) cx1;
      c->pvertices[c->num_vertices].cy1 = (nk_tt_int16) cy1;
   }
   c->num_vertices++;
}

static void nk_tt__csctx_close_shape(nk_tt__csctx *ctx)
{
   if (ctx->first_x != ctx->x || ctx->first_y != ctx->y)
      nk_tt__csctx_v(ctx, NK_TT_vline, (int)ctx->first_x, (int)ctx->first_y, 0, 0, 0, 0);
}

static void nk_tt__csctx_rmove_to(nk_tt__csctx *ctx, float dx, float dy)
{
   nk_tt__csctx_close_shape(ctx);
   ctx->first_x = ctx->x = ctx->x + dx;
   ctx->first_y = ctx->y = ctx->y + dy;
   nk_tt__csctx_v(ctx, NK_TT_vmove, (int)ctx->x, (int)ctx->y, 0, 0, 0, 0);
}

static void nk_tt__csctx_rline_to(nk_tt__csctx *ctx, float dx, float dy)
{
   ctx->x += dx;
   ctx->y += dy;
   nk_tt__csctx_v(ctx, NK_TT_vline, (int)ctx->x, (int)ctx->y, 0, 0, 0, 0);
}

static void nk_tt__csctx_rccurve_to(nk_tt__csctx *ctx, float dx1, float dy1, float dx2, float dy2, float dx3, float dy3)
{
   float cx1 = ctx->x + dx1;
   float cy1 = ctx->y + dy1;
   float cx2 = cx1 + dx2;
   float cy2 = cy1 + dy2;
   ctx->x = cx2 + dx3;
   ctx->y = cy2 + dy3;
   nk_tt__csctx_v(ctx, NK_TT_vcubic, (int)ctx->x, (int)ctx->y, (int)cx1, (int)cy1, (int)cx2, (int)cy2);
}

static nk_tt__buf nk_tt__get_subr(nk_tt__buf idx, int n)
{
   int count = nk_tt__cff_index_count(&idx);
   int bias = 107;
   if (count >= 33900)
      bias = 32768;
   else if (count >= 1240)
      bias = 1131;
   n += bias;
   if (n < 0 || n >= count)
      return nk_tt__new_buf(NULL, 0);
   return nk_tt__cff_index_get(idx, n);
}

static nk_tt__buf nk_tt__cid_get_glyph_subrs(const nk_tt_fontinfo *info, int glyph_index)
{
   nk_tt__buf fdselect = info->fdselect;
   int nranges, start, end, v, fmt, fdselector = -1, i;

   nk_tt__buf_seek(&fdselect, 0);
   fmt = nk_tt__buf_get8(&fdselect);
   if (fmt == 0) {
      // untested
      nk_tt__buf_skip(&fdselect, glyph_index);
      fdselector = nk_tt__buf_get8(&fdselect);
   } else if (fmt == 3) {
      nranges = nk_tt__buf_get16(&fdselect);
      start = nk_tt__buf_get16(&fdselect);
      for (i = 0; i < nranges; i++) {
         v = nk_tt__buf_get8(&fdselect);
         end = nk_tt__buf_get16(&fdselect);
         if (glyph_index >= start && glyph_index < end) {
            fdselector = v;
            break;
         }
         start = end;
      }
   }
   if (fdselector == -1) nk_tt__new_buf(NULL, 0);
   return nk_tt__get_subrs(info->cff, nk_tt__cff_index_get(info->fontdicts, fdselector));
}

static int nk_tt__run_charstring(const nk_tt_fontinfo *info, int glyph_index, nk_tt__csctx *c)
{
   int in_header = 1, maskbits = 0, subr_stack_height = 0, sp = 0, v, i, b0;
   int has_subrs = 0, clear_stack;
   float s[48];
   nk_tt__buf subr_stack[10], subrs = info->subrs, b;
   float f;

#define NK_TT__CSERR(s) (0)

   // this currently ignores the initial width value, which isn't needed if we have hmtx
   b = nk_tt__cff_index_get(info->charstrings, glyph_index);
   while (b.cursor < b.size) {
      i = 0;
      clear_stack = 1;
      b0 = nk_tt__buf_get8(&b);
      switch (b0) {
      // @TODO implement hinting
      case 0x13: // hintmask
      case 0x14: // cntrmask
         if (in_header)
            maskbits += (sp / 2); // implicit "vstem"
         in_header = 0;
         nk_tt__buf_skip(&b, (maskbits + 7) / 8);
         break;

      case 0x01: // hstem
      case 0x03: // vstem
      case 0x12: // hstemhm
      case 0x17: // vstemhm
         maskbits += (sp / 2);
         break;

      case 0x15: // rmoveto
         in_header = 0;
         if (sp < 2) return NK_TT__CSERR("rmoveto stack");
         nk_tt__csctx_rmove_to(c, s[sp-2], s[sp-1]);
         break;
      case 0x04: // vmoveto
         in_header = 0;
         if (sp < 1) return NK_TT__CSERR("vmoveto stack");
         nk_tt__csctx_rmove_to(c, 0, s[sp-1]);
         break;
      case 0x16: // hmoveto
         in_header = 0;
         if (sp < 1) return NK_TT__CSERR("hmoveto stack");
         nk_tt__csctx_rmove_to(c, s[sp-1], 0);
         break;

      case 0x05: // rlineto
         if (sp < 2) return NK_TT__CSERR("rlineto stack");
         for (; i + 1 < sp; i += 2)
            nk_tt__csctx_rline_to(c, s[i], s[i+1]);
         break;

      // hlineto/vlineto and vhcurveto/hvcurveto alternate horizontal and vertical
      // starting from a different place.

      case 0x07: // vlineto
         if (sp < 1) return NK_TT__CSERR("vlineto stack");
         goto vlineto;
      case 0x06: // hlineto
         if (sp < 1) return NK_TT__CSERR("hlineto stack");
         for (;;) {
            if (i >= sp) break;
            nk_tt__csctx_rline_to(c, s[i], 0);
            i++;
      vlineto:
            if (i >= sp) break;
            nk_tt__csctx_rline_to(c, 0, s[i]);
            i++;
         }
         break;

      case 0x1F: // hvcurveto
         if (sp < 4) return NK_TT__CSERR("hvcurveto stack");
         goto hvcurveto;
      case 0x1E: // vhcurveto
         if (sp < 4) return NK_TT__CSERR("vhcurveto stack");
         for (;;) {
            if (i + 3 >= sp) break;
            nk_tt__csctx_rccurve_to(c, 0, s[i], s[i+1], s[i+2], s[i+3], (sp - i == 5) ? s[i + 4] : 0.0f);
            i += 4;
      hvcurveto:
            if (i + 3 >= sp) break;
            nk_tt__csctx_rccurve_to(c, s[i], 0, s[i+1], s[i+2], (sp - i == 5) ? s[i+4] : 0.0f, s[i+3]);
            i += 4;
         }
         break;

      case 0x08: // rrcurveto
         if (sp < 6) return NK_TT__CSERR("rcurveline stack");
         for (; i + 5 < sp; i += 6)
            nk_tt__csctx_rccurve_to(c, s[i], s[i+1], s[i+2], s[i+3], s[i+4], s[i+5]);
         break;

      case 0x18: // rcurveline
         if (sp < 8) return NK_TT__CSERR("rcurveline stack");
         for (; i + 5 < sp - 2; i += 6)
            nk_tt__csctx_rccurve_to(c, s[i], s[i+1], s[i+2], s[i+3], s[i+4], s[i+5]);
         if (i + 1 >= sp) return NK_TT__CSERR("rcurveline stack");
         nk_tt__csctx_rline_to(c, s[i], s[i+1]);
         break;

      case 0x19: // rlinecurve
         if (sp < 8) return NK_TT__CSERR("rlinecurve stack");
         for (; i + 1 < sp - 6; i += 2)
            nk_tt__csctx_rline_to(c, s[i], s[i+1]);
         if (i + 5 >= sp) return NK_TT__CSERR("rlinecurve stack");
         nk_tt__csctx_rccurve_to(c, s[i], s[i+1], s[i+2], s[i+3], s[i+4], s[i+5]);
         break;

      case 0x1A: // vvcurveto
      case 0x1B: // hhcurveto
         if (sp < 4) return NK_TT__CSERR("(vv|hh)curveto stack");
         f = 0.0;
         if (sp & 1) { f = s[i]; i++; }
         for (; i + 3 < sp; i += 4) {
            if (b0 == 0x1B)
               nk_tt__csctx_rccurve_to(c, s[i], f, s[i+1], s[i+2], s[i+3], 0.0);
            else
               nk_tt__csctx_rccurve_to(c, f, s[i], s[i+1], s[i+2], 0.0, s[i+3]);
            f = 0.0;
         }
         break;

      case 0x0A: // callsubr
         if (!has_subrs) {
            if (info->fdselect.size)
               subrs = nk_tt__cid_get_glyph_subrs(info, glyph_index);
            has_subrs = 1;
         }
         // fallthrough
      case 0x1D: // callgsubr
         if (sp < 1) return NK_TT__CSERR("call(g|)subr stack");
         v = (int) s[--sp];
         if (subr_stack_height >= 10) return NK_TT__CSERR("recursion limit");
         subr_stack[subr_stack_height++] = b;
         b = nk_tt__get_subr(b0 == 0x0A ? subrs : info->gsubrs, v);
         if (b.size == 0) return NK_TT__CSERR("subr not found");
         b.cursor = 0;
         clear_stack = 0;
         break;

      case 0x0B: // return
         if (subr_stack_height <= 0) return NK_TT__CSERR("return outside subr");
         b = subr_stack[--subr_stack_height];
         clear_stack = 0;
         break;

      case 0x0E: // endchar
         nk_tt__csctx_close_shape(c);
         return 1;

      case 0x0C: { // two-byte escape
         float dx1, dx2, dx3, dx4, dx5, dx6, dy1, dy2, dy3, dy4, dy5, dy6;
         float dx, dy;
         int b1 = nk_tt__buf_get8(&b);
         switch (b1) {
         // @TODO These "flex" implementations ignore the flex-depth and resolution,
         // and always draw beziers.
         case 0x22: // hflex
            if (sp < 7) return NK_TT__CSERR("hflex stack");
            dx1 = s[0];
            dx2 = s[1];
            dy2 = s[2];
            dx3 = s[3];
            dx4 = s[4];
            dx5 = s[5];
            dx6 = s[6];
            nk_tt__csctx_rccurve_to(c, dx1, 0, dx2, dy2, dx3, 0);
            nk_tt__csctx_rccurve_to(c, dx4, 0, dx5, -dy2, dx6, 0);
            break;

         case 0x23: // flex
            if (sp < 13) return NK_TT__CSERR("flex stack");
            dx1 = s[0];
            dy1 = s[1];
            dx2 = s[2];
            dy2 = s[3];
            dx3 = s[4];
            dy3 = s[5];
            dx4 = s[6];
            dy4 = s[7];
            dx5 = s[8];
            dy5 = s[9];
            dx6 = s[10];
            dy6 = s[11];
            //fd is s[12]
            nk_tt__csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, dy3);
            nk_tt__csctx_rccurve_to(c, dx4, dy4, dx5, dy5, dx6, dy6);
            break;

         case 0x24: // hflex1
            if (sp < 9) return NK_TT__CSERR("hflex1 stack");
            dx1 = s[0];
            dy1 = s[1];
            dx2 = s[2];
            dy2 = s[3];
            dx3 = s[4];
            dx4 = s[5];
            dx5 = s[6];
            dy5 = s[7];
            dx6 = s[8];
            nk_tt__csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, 0);
            nk_tt__csctx_rccurve_to(c, dx4, 0, dx5, dy5, dx6, -(dy1+dy2+dy5));
            break;

         case 0x25: // flex1
            if (sp < 11) return NK_TT__CSERR("flex1 stack");
            dx1 = s[0];
            dy1 = s[1];
            dx2 = s[2];
            dy2 = s[3];
            dx3 = s[4];
            dy3 = s[5];
            dx4 = s[6];
            dy4 = s[7];
            dx5 = s[8];
            dy5 = s[9];
            dx6 = dy6 = s[10];
            dx = dx1+dx2+dx3+dx4+dx5;
            dy = dy1+dy2+dy3+dy4+dy5;
            if (NK_TT_fabs(dx) > NK_TT_fabs(dy))
               dy6 = -dy;
            else
               dx6 = -dx;
            nk_tt__csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, dy3);
            nk_tt__csctx_rccurve_to(c, dx4, dy4, dx5, dy5, dx6, dy6);
            break;

         default:
            return NK_TT__CSERR("unimplemented");
         }
      } break;

      default:
         if (b0 != 255 && b0 != 28 && (b0 < 32 || b0 > 254))
            return NK_TT__CSERR("reserved operator");

         // push immediate
         if (b0 == 255) {
            f = (float)(nk_tt_int32)nk_tt__buf_get32(&b) / 0x10000;
         } else {
            nk_tt__buf_skip(&b, -1);
            f = (float)(nk_tt_int16)nk_tt__cff_int(&b);
         }
         if (sp >= 48) return NK_TT__CSERR("push stack overflow");
         s[sp++] = f;
         clear_stack = 0;
         break;
      }
      if (clear_stack) sp = 0;
   }
   return NK_TT__CSERR("no endchar");

#undef NK_TT__CSERR
}

static int nk_tt__GetGlyphShapeT2(const nk_tt_fontinfo *info, int glyph_index, nk_tt_vertex **pvertices)
{
   // runs the charstring twice, once to count and once to output (to avoid realloc)
   nk_tt__csctx count_ctx = NK_TT__CSCTX_INIT(1);
   nk_tt__csctx output_ctx = NK_TT__CSCTX_INIT(0);
   if (nk_tt__run_charstring(info, glyph_index, &count_ctx)) {
      *pvertices = (nk_tt_vertex*)NK_TT_malloc(count_ctx.num_vertices*sizeof(nk_tt_vertex), info->userdata);
      output_ctx.pvertices = *pvertices;
      if (nk_tt__run_charstring(info, glyph_index, &output_ctx)) {
         NK_TT_assert(output_ctx.num_vertices == count_ctx.num_vertices);
         return output_ctx.num_vertices;
      }
   }
   *pvertices = NULL;
   return 0;
}

static int nk_tt__GetGlyphInfoT2(const nk_tt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1)
{
   nk_tt__csctx c = NK_TT__CSCTX_INIT(1);
   int r = nk_tt__run_charstring(info, glyph_index, &c);
   if (x0)  *x0 = r ? c.min_x : 0;
   if (y0)  *y0 = r ? c.min_y : 0;
   if (x1)  *x1 = r ? c.max_x : 0;
   if (y1)  *y1 = r ? c.max_y : 0;
   return r ? c.num_vertices : 0;
}

NK_TT_DEF int nk_tt_GetGlyphShape(const nk_tt_fontinfo *info, int glyph_index, nk_tt_vertex **pvertices)
{
   if (!info->cff.size)
      return nk_tt__GetGlyphShapeTT(info, glyph_index, pvertices);
   else
      return nk_tt__GetGlyphShapeT2(info, glyph_index, pvertices);
}

NK_TT_DEF void nk_tt_GetGlyphHMetrics(const nk_tt_fontinfo *info, int glyph_index, int *advanceWidth, int *leftSideBearing)
{
   nk_tt_uint16 numOfLongHorMetrics = ttUSHORT(info->data+info->hhea + 34);
   if (glyph_index < numOfLongHorMetrics) {
      if (advanceWidth)     *advanceWidth    = ttSHORT(info->data + info->hmtx + 4*glyph_index);
      if (leftSideBearing)  *leftSideBearing = ttSHORT(info->data + info->hmtx + 4*glyph_index + 2);
   } else {
      if (advanceWidth)     *advanceWidth    = ttSHORT(info->data + info->hmtx + 4*(numOfLongHorMetrics-1));
      if (leftSideBearing)  *leftSideBearing = ttSHORT(info->data + info->hmtx + 4*numOfLongHorMetrics + 2*(glyph_index - numOfLongHorMetrics));
   }
}

static int  nk_tt__GetGlyphKernInfoAdvance(const nk_tt_fontinfo *info, int glyph1, int glyph2)
{
   nk_tt_uint8 *data = info->data + info->kern;
   nk_tt_uint32 needle, straw;
   int l, r, m;

   // we only look at the first table. it must be 'horizontal' and format 0.
   if (!info->kern)
      return 0;
   if (ttUSHORT(data+2) < 1) // number of tables, need at least 1
      return 0;
   if (ttUSHORT(data+8) != 1) // horizontal flag must be set in format
      return 0;

   l = 0;
   r = ttUSHORT(data+10) - 1;
   needle = glyph1 << 16 | glyph2;
   while (l <= r) {
      m = (l + r) >> 1;
      straw = ttULONG(data+18+(m*6)); // note: unaligned read
      if (needle < straw)
         r = m - 1;
      else if (needle > straw)
         l = m + 1;
      else
         return ttSHORT(data+22+(m*6));
   }
   return 0;
}

static nk_tt_int32  nk_tt__GetCoverageIndex(nk_tt_uint8 *coverageTable, int glyph)
{
    nk_tt_uint16 coverageFormat = ttUSHORT(coverageTable);
    switch(coverageFormat) {
        case 1: {
            nk_tt_uint16 glyphCount = ttUSHORT(coverageTable + 2);

            // Binary search.
            nk_tt_int32 l=0, r=glyphCount-1, m;
            int straw, needle=glyph;
            while (l <= r) {
                nk_tt_uint8 *glyphArray = coverageTable + 4;
                nk_tt_uint16 glyphID;
                m = (l + r) >> 1;
                glyphID = ttUSHORT(glyphArray + 2 * m);
                straw = glyphID;
                if (needle < straw)
                    r = m - 1;
                else if (needle > straw)
                    l = m + 1;
                else {
                     return m;
                }
            }
        } break;

        case 2: {
            nk_tt_uint16 rangeCount = ttUSHORT(coverageTable + 2);
            nk_tt_uint8 *rangeArray = coverageTable + 4;

            // Binary search.
            nk_tt_int32 l=0, r=rangeCount-1, m;
            int strawStart, strawEnd, needle=glyph;
            while (l <= r) {
                nk_tt_uint8 *rangeRecord;
                m = (l + r) >> 1;
                rangeRecord = rangeArray + 6 * m;
                strawStart = ttUSHORT(rangeRecord);
                strawEnd = ttUSHORT(rangeRecord + 2);
                if (needle < strawStart)
                    r = m - 1;
                else if (needle > strawEnd)
                    l = m + 1;
                else {
                    nk_tt_uint16 startCoverageIndex = ttUSHORT(rangeRecord + 4);
                    return startCoverageIndex + glyph - strawStart;
                }
            }
        } break;

        default: {
            // There are no other cases.
            NK_TT_assert(0);
        } break;
    }

    return -1;
}

static nk_tt_int32  nk_tt__GetGlyphClass(nk_tt_uint8 *classDefTable, int glyph)
{
    nk_tt_uint16 classDefFormat = ttUSHORT(classDefTable);
    switch(classDefFormat)
    {
        case 1: {
            nk_tt_uint16 startGlyphID = ttUSHORT(classDefTable + 2);
            nk_tt_uint16 glyphCount = ttUSHORT(classDefTable + 4);
            nk_tt_uint8 *classDef1ValueArray = classDefTable + 6;

            if (glyph >= startGlyphID && glyph < startGlyphID + glyphCount)
                return (nk_tt_int32)ttUSHORT(classDef1ValueArray + 2 * (glyph - startGlyphID));

            classDefTable = classDef1ValueArray + 2 * glyphCount;
        } break;

        case 2: {
            nk_tt_uint16 classRangeCount = ttUSHORT(classDefTable + 2);
            nk_tt_uint8 *classRangeRecords = classDefTable + 4;

            // Binary search.
            nk_tt_int32 l=0, r=classRangeCount-1, m;
            int strawStart, strawEnd, needle=glyph;
            while (l <= r) {
                nk_tt_uint8 *classRangeRecord;
                m = (l + r) >> 1;
                classRangeRecord = classRangeRecords + 6 * m;
                strawStart = ttUSHORT(classRangeRecord);
                strawEnd = ttUSHORT(classRangeRecord + 2);
                if (needle < strawStart)
                    r = m - 1;
                else if (needle > strawEnd)
                    l = m + 1;
                else
                    return (nk_tt_int32)ttUSHORT(classRangeRecord + 4);
            }

            classDefTable = classRangeRecords + 6 * classRangeCount;
        } break;

        default: {
            // There are no other cases.
            NK_TT_assert(0);
        } break;
    }

    return -1;
}

// Define to NK_TT_assert(x) if you want to break on unimplemented formats.
#define NK_TT_GPOS_TODO_assert(x)

static nk_tt_int32  nk_tt__GetGlyphGPOSInfoAdvance(const nk_tt_fontinfo *info, int glyph1, int glyph2)
{
    nk_tt_uint16 lookupListOffset;
    nk_tt_uint8 *lookupList;
    nk_tt_uint16 lookupCount;
    nk_tt_uint8 *data;
    nk_tt_int32 i;

    if (!info->gpos) return 0;

    data = info->data + info->gpos;

    if (ttUSHORT(data+0) != 1) return 0; // Major version 1
    if (ttUSHORT(data+2) != 0) return 0; // Minor version 0

    lookupListOffset = ttUSHORT(data+8);
    lookupList = data + lookupListOffset;
    lookupCount = ttUSHORT(lookupList);

    for (i=0; i<lookupCount; ++i) {
        nk_tt_uint16 lookupOffset = ttUSHORT(lookupList + 2 + 2 * i);
        nk_tt_uint8 *lookupTable = lookupList + lookupOffset;

        nk_tt_uint16 lookupType = ttUSHORT(lookupTable);
        nk_tt_uint16 subTableCount = ttUSHORT(lookupTable + 4);
        nk_tt_uint8 *subTableOffsets = lookupTable + 6;
        switch(lookupType) {
            case 2: { // Pair Adjustment Positioning Subtable
                nk_tt_int32 sti;
                for (sti=0; sti<subTableCount; sti++) {
                    nk_tt_uint16 subtableOffset = ttUSHORT(subTableOffsets + 2 * sti);
                    nk_tt_uint8 *table = lookupTable + subtableOffset;
                    nk_tt_uint16 posFormat = ttUSHORT(table);
                    nk_tt_uint16 coverageOffset = ttUSHORT(table + 2);
                    nk_tt_int32 coverageIndex = nk_tt__GetCoverageIndex(table + coverageOffset, glyph1);
                    if (coverageIndex == -1) continue;

                    switch (posFormat) {
                        case 1: {
                            nk_tt_int32 l, r, m;
                            int straw, needle;
                            nk_tt_uint16 valueFormat1 = ttUSHORT(table + 4);
                            nk_tt_uint16 valueFormat2 = ttUSHORT(table + 6);
                            nk_tt_int32 valueRecordPairSizeInBytes = 2;
                            nk_tt_uint16 pairSetCount = ttUSHORT(table + 8);
                            nk_tt_uint16 pairPosOffset = ttUSHORT(table + 10 + 2 * coverageIndex);
                            nk_tt_uint8 *pairValueTable = table + pairPosOffset;
                            nk_tt_uint16 pairValueCount = ttUSHORT(pairValueTable);
                            nk_tt_uint8 *pairValueArray = pairValueTable + 2;
                            // TODO: Support more formats.
                            NK_TT_GPOS_TODO_assert(valueFormat1 == 4);
                            if (valueFormat1 != 4) return 0;
                            NK_TT_GPOS_TODO_assert(valueFormat2 == 0);
                            if (valueFormat2 != 0) return 0;

                            NK_TT_assert(coverageIndex < pairSetCount);
                            NK_TT__NOTUSED(pairSetCount);

                            needle=glyph2;
                            r=pairValueCount-1;
                            l=0;

                            // Binary search.
                            while (l <= r) {
                                nk_tt_uint16 secondGlyph;
                                nk_tt_uint8 *pairValue;
                                m = (l + r) >> 1;
                                pairValue = pairValueArray + (2 + valueRecordPairSizeInBytes) * m;
                                secondGlyph = ttUSHORT(pairValue);
                                straw = secondGlyph;
                                if (needle < straw)
                                    r = m - 1;
                                else if (needle > straw)
                                    l = m + 1;
                                else {
                                    nk_tt_int16 xAdvance = ttSHORT(pairValue + 2);
                                    return xAdvance;
                                }
                            }
                        } break;

                        case 2: {
                            nk_tt_uint16 valueFormat1 = ttUSHORT(table + 4);
                            nk_tt_uint16 valueFormat2 = ttUSHORT(table + 6);

                            nk_tt_uint16 classDef1Offset = ttUSHORT(table + 8);
                            nk_tt_uint16 classDef2Offset = ttUSHORT(table + 10);
                            int glyph1class = nk_tt__GetGlyphClass(table + classDef1Offset, glyph1);
                            int glyph2class = nk_tt__GetGlyphClass(table + classDef2Offset, glyph2);

                            nk_tt_uint16 class1Count = ttUSHORT(table + 12);
                            nk_tt_uint16 class2Count = ttUSHORT(table + 14);
                            NK_TT_assert(glyph1class < class1Count);
                            NK_TT_assert(glyph2class < class2Count);

                            // TODO: Support more formats.
                            NK_TT_GPOS_TODO_assert(valueFormat1 == 4);
                            if (valueFormat1 != 4) return 0;
                            NK_TT_GPOS_TODO_assert(valueFormat2 == 0);
                            if (valueFormat2 != 0) return 0;

                            if (glyph1class >= 0 && glyph1class < class1Count && glyph2class >= 0 && glyph2class < class2Count) {
                                nk_tt_uint8 *class1Records = table + 16;
                                nk_tt_uint8 *class2Records = class1Records + 2 * (glyph1class * class2Count);
                                nk_tt_int16 xAdvance = ttSHORT(class2Records + 2 * glyph2class);
                                return xAdvance;
                            }
                        } break;

                        default: {
                            // There are no other cases.
                            NK_TT_assert(0);
                            break;
                        };
                    }
                }
                break;
            };

            default:
                // TODO: Implement other stuff.
                break;
        }
    }

    return 0;
}

NK_TT_DEF int  nk_tt_GetGlyphKernAdvance(const nk_tt_fontinfo *info, int g1, int g2)
{
   int xAdvance = 0;

   if (info->gpos)
      xAdvance += nk_tt__GetGlyphGPOSInfoAdvance(info, g1, g2);
   else if (info->kern)
      xAdvance += nk_tt__GetGlyphKernInfoAdvance(info, g1, g2);

   return xAdvance;
}

NK_TT_DEF int  nk_tt_GetCodepointKernAdvance(const nk_tt_fontinfo *info, int ch1, int ch2)
{
   if (!info->kern && !info->gpos) // if no kerning table, don't waste time looking up both codepoint->glyphs
      return 0;
   return nk_tt_GetGlyphKernAdvance(info, nk_tt_FindGlyphIndex(info,ch1), nk_tt_FindGlyphIndex(info,ch2));
}

NK_TT_DEF void nk_tt_GetCodepointHMetrics(const nk_tt_fontinfo *info, int codepoint, int *advanceWidth, int *leftSideBearing)
{
   nk_tt_GetGlyphHMetrics(info, nk_tt_FindGlyphIndex(info,codepoint), advanceWidth, leftSideBearing);
}

NK_TT_DEF void nk_tt_GetFontVMetrics(const nk_tt_fontinfo *info, int *ascent, int *descent, int *lineGap)
{
   if (ascent ) *ascent  = ttSHORT(info->data+info->hhea + 4);
   if (descent) *descent = ttSHORT(info->data+info->hhea + 6);
   if (lineGap) *lineGap = ttSHORT(info->data+info->hhea + 8);
}

NK_TT_DEF int  nk_tt_GetFontVMetricsOS2(const nk_tt_fontinfo *info, int *typoAscent, int *typoDescent, int *typoLineGap)
{
   int tab = nk_tt__find_table(info->data, info->fontstart, "OS/2");
   if (!tab)
      return 0;
   if (typoAscent ) *typoAscent  = ttSHORT(info->data+tab + 68);
   if (typoDescent) *typoDescent = ttSHORT(info->data+tab + 70);
   if (typoLineGap) *typoLineGap = ttSHORT(info->data+tab + 72);
   return 1;
}

NK_TT_DEF void nk_tt_GetFontBoundingBox(const nk_tt_fontinfo *info, int *x0, int *y0, int *x1, int *y1)
{
   *x0 = ttSHORT(info->data + info->head + 36);
   *y0 = ttSHORT(info->data + info->head + 38);
   *x1 = ttSHORT(info->data + info->head + 40);
   *y1 = ttSHORT(info->data + info->head + 42);
}

NK_TT_DEF float nk_tt_ScaleForPixelHeight(const nk_tt_fontinfo *info, float height)
{
   int fheight = ttSHORT(info->data + info->hhea + 4) - ttSHORT(info->data + info->hhea + 6);
   return (float) height / fheight;
}

NK_TT_DEF float nk_tt_ScaleForMappingEmToPixels(const nk_tt_fontinfo *info, float pixels)
{
   int unitsPerEm = ttUSHORT(info->data + info->head + 18);
   return pixels / unitsPerEm;
}

NK_TT_DEF void nk_tt_FreeShape(const nk_tt_fontinfo *info, nk_tt_vertex *v)
{
   NK_TT_free(v, info->userdata);
}

//////////////////////////////////////////////////////////////////////////////
//
// antialiasing software rasterizer
//

NK_TT_DEF void nk_tt_GetGlyphBitmapBoxSubpixel(const nk_tt_fontinfo *font, int glyph, float scale_x, float scale_y,float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1)
{
   int x0=0,y0=0,x1,y1; // =0 suppresses compiler warning
   if (!nk_tt_GetGlyphBox(font, glyph, &x0,&y0,&x1,&y1)) {
      // e.g. space character
      if (ix0) *ix0 = 0;
      if (iy0) *iy0 = 0;
      if (ix1) *ix1 = 0;
      if (iy1) *iy1 = 0;
   } else {
      // move to integral bboxes (treating pixels as little squares, what pixels get touched)?
      if (ix0) *ix0 = NK_TT_ifloor( x0 * scale_x + shift_x);
      if (iy0) *iy0 = NK_TT_ifloor(-y1 * scale_y + shift_y);
      if (ix1) *ix1 = NK_TT_iceil ( x1 * scale_x + shift_x);
      if (iy1) *iy1 = NK_TT_iceil (-y0 * scale_y + shift_y);
   }
}

NK_TT_DEF void nk_tt_GetGlyphBitmapBox(const nk_tt_fontinfo *font, int glyph, float scale_x, float scale_y, int *ix0, int *iy0, int *ix1, int *iy1)
{
   nk_tt_GetGlyphBitmapBoxSubpixel(font, glyph, scale_x, scale_y,0.0f,0.0f, ix0, iy0, ix1, iy1);
}

NK_TT_DEF void nk_tt_GetCodepointBitmapBoxSubpixel(const nk_tt_fontinfo *font, int codepoint, float scale_x, float scale_y, float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1)
{
   nk_tt_GetGlyphBitmapBoxSubpixel(font, nk_tt_FindGlyphIndex(font,codepoint), scale_x, scale_y,shift_x,shift_y, ix0,iy0,ix1,iy1);
}

NK_TT_DEF void nk_tt_GetCodepointBitmapBox(const nk_tt_fontinfo *font, int codepoint, float scale_x, float scale_y, int *ix0, int *iy0, int *ix1, int *iy1)
{
   nk_tt_GetCodepointBitmapBoxSubpixel(font, codepoint, scale_x, scale_y,0.0f,0.0f, ix0,iy0,ix1,iy1);
}

//////////////////////////////////////////////////////////////////////////////
//
//  Rasterizer

typedef struct nk_tt__hheap_chunk
{
   struct nk_tt__hheap_chunk *next;
} nk_tt__hheap_chunk;

typedef struct nk_tt__hheap
{
   struct nk_tt__hheap_chunk *head;
   void   *first_free;
   int    num_remaining_in_head_chunk;
} nk_tt__hheap;

static void *nk_tt__hheap_alloc(nk_tt__hheap *hh, size_t size, void *userdata)
{
   if (hh->first_free) {
      void *p = hh->first_free;
      hh->first_free = * (void **) p;
      return p;
   } else {
      if (hh->num_remaining_in_head_chunk == 0) {
         int count = (size < 32 ? 2000 : size < 128 ? 800 : 100);
         nk_tt__hheap_chunk *c = (nk_tt__hheap_chunk *) NK_TT_malloc(sizeof(nk_tt__hheap_chunk) + size * count, userdata);
         if (c == NULL)
            return NULL;
         c->next = hh->head;
         hh->head = c;
         hh->num_remaining_in_head_chunk = count;
      }
      --hh->num_remaining_in_head_chunk;
      return (char *) (hh->head) + sizeof(nk_tt__hheap_chunk) + size * hh->num_remaining_in_head_chunk;
   }
}

static void nk_tt__hheap_free(nk_tt__hheap *hh, void *p)
{
   *(void **) p = hh->first_free;
   hh->first_free = p;
}

static void nk_tt__hheap_cleanup(nk_tt__hheap *hh, void *userdata)
{
   nk_tt__hheap_chunk *c = hh->head;
   while (c) {
      nk_tt__hheap_chunk *n = c->next;
      NK_TT_free(c, userdata);
      c = n;
   }
}

typedef struct nk_tt__edge {
   float x0,y0, x1,y1;
   int invert;
} nk_tt__edge;


typedef struct nk_tt__active_edge
{
   struct nk_tt__active_edge *next;
   #if NK_TT_RASTERIZER_VERSION==1
   int x,dx;
   float ey;
   int direction;
   #elif NK_TT_RASTERIZER_VERSION==2
   float fx,fdx,fdy;
   float direction;
   float sy;
   float ey;
   #else
   #error "Unrecognized value of NK_TT_RASTERIZER_VERSION"
   #endif
} nk_tt__active_edge;

#if NK_TT_RASTERIZER_VERSION == 1
#define NK_TT_FIXSHIFT   10
#define NK_TT_FIX        (1 << NK_TT_FIXSHIFT)
#define NK_TT_FIXMASK    (NK_TT_FIX-1)

static nk_tt__active_edge *nk_tt__new_active(nk_tt__hheap *hh, nk_tt__edge *e, int off_x, float start_point, void *userdata)
{
   nk_tt__active_edge *z = (nk_tt__active_edge *) nk_tt__hheap_alloc(hh, sizeof(*z), userdata);
   float dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
   NK_TT_assert(z != NULL);
   if (!z) return z;
   
   // round dx down to avoid overshooting
   if (dxdy < 0)
      z->dx = -NK_TT_ifloor(NK_TT_FIX * -dxdy);
   else
      z->dx = NK_TT_ifloor(NK_TT_FIX * dxdy);

   z->x = NK_TT_ifloor(NK_TT_FIX * e->x0 + z->dx * (start_point - e->y0)); // use z->dx so when we offset later it's by the same amount
   z->x -= off_x * NK_TT_FIX;

   z->ey = e->y1;
   z->next = 0;
   z->direction = e->invert ? 1 : -1;
   return z;
}
#elif NK_TT_RASTERIZER_VERSION == 2
static nk_tt__active_edge *nk_tt__new_active(nk_tt__hheap *hh, nk_tt__edge *e, int off_x, float start_point, void *userdata)
{
   nk_tt__active_edge *z = (nk_tt__active_edge *) nk_tt__hheap_alloc(hh, sizeof(*z), userdata);
   float dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
   NK_TT_assert(z != NULL);
   //NK_TT_assert(e->y0 <= start_point);
   if (!z) return z;
   z->fdx = dxdy;
   z->fdy = dxdy != 0.0f ? (1.0f/dxdy) : 0.0f;
   z->fx = e->x0 + dxdy * (start_point - e->y0);
   z->fx -= off_x;
   z->direction = e->invert ? 1.0f : -1.0f;
   z->sy = e->y0;
   z->ey = e->y1;
   z->next = 0;
   return z;
}
#else
#error "Unrecognized value of NK_TT_RASTERIZER_VERSION"
#endif

#if NK_TT_RASTERIZER_VERSION == 1
// note: this routine clips fills that extend off the edges... ideally this
// wouldn't happen, but it could happen if the truetype glyph bounding boxes
// are wrong, or if the user supplies a too-small bitmap
static void nk_tt__fill_active_edges(unsigned char *scanline, int len, nk_tt__active_edge *e, int max_weight)
{
   // non-zero winding fill
   int x0=0, w=0;

   while (e) {
      if (w == 0) {
         // if we're currently at zero, we need to record the edge start point
         x0 = e->x; w += e->direction;
      } else {
         int x1 = e->x; w += e->direction;
         // if we went to zero, we need to draw
         if (w == 0) {
            int i = x0 >> NK_TT_FIXSHIFT;
            int j = x1 >> NK_TT_FIXSHIFT;

            if (i < len && j >= 0) {
               if (i == j) {
                  // x0,x1 are the same pixel, so compute combined coverage
                  scanline[i] = scanline[i] + (nk_tt_uint8) ((x1 - x0) * max_weight >> NK_TT_FIXSHIFT);
               } else {
                  if (i >= 0) // add antialiasing for x0
                     scanline[i] = scanline[i] + (nk_tt_uint8) (((NK_TT_FIX - (x0 & NK_TT_FIXMASK)) * max_weight) >> NK_TT_FIXSHIFT);
                  else
                     i = -1; // clip

                  if (j < len) // add antialiasing for x1
                     scanline[j] = scanline[j] + (nk_tt_uint8) (((x1 & NK_TT_FIXMASK) * max_weight) >> NK_TT_FIXSHIFT);
                  else
                     j = len; // clip

                  for (++i; i < j; ++i) // fill pixels between x0 and x1
                     scanline[i] = scanline[i] + (nk_tt_uint8) max_weight;
               }
            }
         }
      }
      
      e = e->next;
   }
}

static void nk_tt__rasterize_sorted_edges(nk_tt__bitmap *result, nk_tt__edge *e, int n, int vsubsample, int off_x, int off_y, void *userdata)
{
   nk_tt__hheap hh = { 0, 0, 0 };
   nk_tt__active_edge *active = NULL;
   int y,j=0;
   int max_weight = (255 / vsubsample);  // weight per vertical scanline
   int s; // vertical subsample index
   unsigned char scanline_data[512], *scanline;

   if (result->w > 512)
      scanline = (unsigned char *) NK_TT_malloc(result->w, userdata);
   else
      scanline = scanline_data;

   y = off_y * vsubsample;
   e[n].y0 = (off_y + result->h) * (float) vsubsample + 1;

   while (j < result->h) {
      NK_TT_memset(scanline, 0, result->w);
      for (s=0; s < vsubsample; ++s) {
         // find center of pixel for this scanline
         float scan_y = y + 0.5f;
         nk_tt__active_edge **step = &active;

         // update all active edges;
         // remove all active edges that terminate before the center of this scanline
         while (*step) {
            nk_tt__active_edge * z = *step;
            if (z->ey <= scan_y) {
               *step = z->next; // delete from list
               NK_TT_assert(z->direction);
               z->direction = 0;
               nk_tt__hheap_free(&hh, z);
            } else {
               z->x += z->dx; // advance to position for current scanline
               step = &((*step)->next); // advance through list
            }
         }

         // resort the list if needed
         for(;;) {
            int changed=0;
            step = &active;
            while (*step && (*step)->next) {
               if ((*step)->x > (*step)->next->x) {
                  nk_tt__active_edge *t = *step;
                  nk_tt__active_edge *q = t->next;

                  t->next = q->next;
                  q->next = t;
                  *step = q;
                  changed = 1;
               }
               step = &(*step)->next;
            }
            if (!changed) break;
         }

         // insert all edges that start before the center of this scanline -- omit ones that also end on this scanline
         while (e->y0 <= scan_y) {
            if (e->y1 > scan_y) {
               nk_tt__active_edge *z = nk_tt__new_active(&hh, e, off_x, scan_y, userdata);
               if (z != NULL) {
                  // find insertion point
                  if (active == NULL)
                     active = z;
                  else if (z->x < active->x) {
                     // insert at front
                     z->next = active;
                     active = z;
                  } else {
                     // find thing to insert AFTER
                     nk_tt__active_edge *p = active;
                     while (p->next && p->next->x < z->x)
                        p = p->next;
                     // at this point, p->next->x is NOT < z->x
                     z->next = p->next;
                     p->next = z;
                  }
               }
            }
            ++e;
         }

         // now process all active edges in XOR fashion
         if (active)
            nk_tt__fill_active_edges(scanline, result->w, active, max_weight);

         ++y;
      }
      NK_TT_memcpy(result->pixels + j * result->stride, scanline, result->w);
      ++j;
   }

   nk_tt__hheap_cleanup(&hh, userdata);

   if (scanline != scanline_data)
      NK_TT_free(scanline, userdata);
}

#elif NK_TT_RASTERIZER_VERSION == 2

// the edge passed in here does not cross the vertical line at x or the vertical line at x+1
// (i.e. it has already been clipped to those)
static void nk_tt__handle_clipped_edge(float *scanline, int x, nk_tt__active_edge *e, float x0, float y0, float x1, float y1)
{
   if (y0 == y1) return;
   NK_TT_assert(y0 < y1);
   NK_TT_assert(e->sy <= e->ey);
   if (y0 > e->ey) return;
   if (y1 < e->sy) return;
   if (y0 < e->sy) {
      x0 += (x1-x0) * (e->sy - y0) / (y1-y0);
      y0 = e->sy;
   }
   if (y1 > e->ey) {
      x1 += (x1-x0) * (e->ey - y1) / (y1-y0);
      y1 = e->ey;
   }

   if (x0 == x)
      NK_TT_assert(x1 <= x+1);
   else if (x0 == x+1)
      NK_TT_assert(x1 >= x);
   else if (x0 <= x)
      NK_TT_assert(x1 <= x);
   else if (x0 >= x+1)
      NK_TT_assert(x1 >= x+1);
   else
      NK_TT_assert(x1 >= x && x1 <= x+1);

   if (x0 <= x && x1 <= x)
      scanline[x] += e->direction * (y1-y0);
   else if (x0 >= x+1 && x1 >= x+1)
      ;
   else {
      NK_TT_assert(x0 >= x && x0 <= x+1 && x1 >= x && x1 <= x+1);
      scanline[x] += e->direction * (y1-y0) * (1-((x0-x)+(x1-x))/2); // coverage = 1 - average x position
   }
}

static void nk_tt__fill_active_edges_new(float *scanline, float *scanline_fill, int len, nk_tt__active_edge *e, float y_top)
{
   float y_bottom = y_top+1;

   while (e) {
      // brute force every pixel

      // compute intersection points with top & bottom
      NK_TT_assert(e->ey >= y_top);

      if (e->fdx == 0) {
         float x0 = e->fx;
         if (x0 < len) {
            if (x0 >= 0) {
               nk_tt__handle_clipped_edge(scanline,(int) x0,e, x0,y_top, x0,y_bottom);
               nk_tt__handle_clipped_edge(scanline_fill-1,(int) x0+1,e, x0,y_top, x0,y_bottom);
            } else {
               nk_tt__handle_clipped_edge(scanline_fill-1,0,e, x0,y_top, x0,y_bottom);
            }
         }
      } else {
         float x0 = e->fx;
         float dx = e->fdx;
         float xb = x0 + dx;
         float x_top, x_bottom;
         float sy0,sy1;
         float dy = e->fdy;
         NK_TT_assert(e->sy <= y_bottom && e->ey >= y_top);

         // compute endpoints of line segment clipped to this scanline (if the
         // line segment starts on this scanline. x0 is the intersection of the
         // line with y_top, but that may be off the line segment.
         if (e->sy > y_top) {
            x_top = x0 + dx * (e->sy - y_top);
            sy0 = e->sy;
         } else {
            x_top = x0;
            sy0 = y_top;
         }
         if (e->ey < y_bottom) {
            x_bottom = x0 + dx * (e->ey - y_top);
            sy1 = e->ey;
         } else {
            x_bottom = xb;
            sy1 = y_bottom;
         }

         if (x_top >= 0 && x_bottom >= 0 && x_top < len && x_bottom < len) {
            // from here on, we don't have to range check x values

            if ((int) x_top == (int) x_bottom) {
               float height;
               // simple case, only spans one pixel
               int x = (int) x_top;
               height = sy1 - sy0;
               NK_TT_assert(x >= 0 && x < len);
               scanline[x] += e->direction * (1-((x_top - x) + (x_bottom-x))/2)  * height;
               scanline_fill[x] += e->direction * height; // everything right of this pixel is filled
            } else {
               int x,x1,x2;
               float y_crossing, step, sign, area;
               // covers 2+ pixels
               if (x_top > x_bottom) {
                  // flip scanline vertically; signed area is the same
                  float t;
                  sy0 = y_bottom - (sy0 - y_top);
                  sy1 = y_bottom - (sy1 - y_top);
                  t = sy0, sy0 = sy1, sy1 = t;
                  t = x_bottom, x_bottom = x_top, x_top = t;
                  dx = -dx;
                  dy = -dy;
                  t = x0, x0 = xb, xb = t;
               }

               x1 = (int) x_top;
               x2 = (int) x_bottom;
               // compute intersection with y axis at x1+1
               y_crossing = (x1+1 - x0) * dy + y_top;

               sign = e->direction;
               // area of the rectangle covered from y0..y_crossing
               area = sign * (y_crossing-sy0);
               // area of the triangle (x_top,y0), (x+1,y0), (x+1,y_crossing)
               scanline[x1] += area * (1-((x_top - x1)+(x1+1-x1))/2);

               step = sign * dy;
               for (x = x1+1; x < x2; ++x) {
                  scanline[x] += area + step/2;
                  area += step;
               }
               y_crossing += dy * (x2 - (x1+1));

               NK_TT_assert(NK_TT_fabs(area) <= 1.01f);

               scanline[x2] += area + sign * (1-((x2-x2)+(x_bottom-x2))/2) * (sy1-y_crossing);

               scanline_fill[x2] += sign * (sy1-sy0);
            }
         } else {
            // if edge goes outside of box we're drawing, we require
            // clipping logic. since this does not match the intended use
            // of this library, we use a different, very slow brute
            // force implementation
            int x;
            for (x=0; x < len; ++x) {
               // cases:
               //
               // there can be up to two intersections with the pixel. any intersection
               // with left or right edges can be handled by splitting into two (or three)
               // regions. intersections with top & bottom do not necessitate case-wise logic.
               //
               // the old way of doing this found the intersections with the left & right edges,
               // then used some simple logic to produce up to three segments in sorted order
               // from top-to-bottom. however, this had a problem: if an x edge was epsilon
               // across the x border, then the corresponding y position might not be distinct
               // from the other y segment, and it might ignored as an empty segment. to avoid
               // that, we need to explicitly produce segments based on x positions.

               // rename variables to clearly-defined pairs
               float y0 = y_top;
               float x1 = (float) (x);
               float x2 = (float) (x+1);
               float x3 = xb;
               float y3 = y_bottom;

               // x = e->x + e->dx * (y-y_top)
               // (y-y_top) = (x - e->x) / e->dx
               // y = (x - e->x) / e->dx + y_top
               float y1 = (x - x0) / dx + y_top;
               float y2 = (x+1 - x0) / dx + y_top;

               if (x0 < x1 && x3 > x2) {         // three segments descending down-right
                  nk_tt__handle_clipped_edge(scanline,x,e, x0,y0, x1,y1);
                  nk_tt__handle_clipped_edge(scanline,x,e, x1,y1, x2,y2);
                  nk_tt__handle_clipped_edge(scanline,x,e, x2,y2, x3,y3);
               } else if (x3 < x1 && x0 > x2) {  // three segments descending down-left
                  nk_tt__handle_clipped_edge(scanline,x,e, x0,y0, x2,y2);
                  nk_tt__handle_clipped_edge(scanline,x,e, x2,y2, x1,y1);
                  nk_tt__handle_clipped_edge(scanline,x,e, x1,y1, x3,y3);
               } else if (x0 < x1 && x3 > x1) {  // two segments across x, down-right
                  nk_tt__handle_clipped_edge(scanline,x,e, x0,y0, x1,y1);
                  nk_tt__handle_clipped_edge(scanline,x,e, x1,y1, x3,y3);
               } else if (x3 < x1 && x0 > x1) {  // two segments across x, down-left
                  nk_tt__handle_clipped_edge(scanline,x,e, x0,y0, x1,y1);
                  nk_tt__handle_clipped_edge(scanline,x,e, x1,y1, x3,y3);
               } else if (x0 < x2 && x3 > x2) {  // two segments across x+1, down-right
                  nk_tt__handle_clipped_edge(scanline,x,e, x0,y0, x2,y2);
                  nk_tt__handle_clipped_edge(scanline,x,e, x2,y2, x3,y3);
               } else if (x3 < x2 && x0 > x2) {  // two segments across x+1, down-left
                  nk_tt__handle_clipped_edge(scanline,x,e, x0,y0, x2,y2);
                  nk_tt__handle_clipped_edge(scanline,x,e, x2,y2, x3,y3);
               } else {  // one segment
                  nk_tt__handle_clipped_edge(scanline,x,e, x0,y0, x3,y3);
               }
            }
         }
      }
      e = e->next;
   }
}

// directly AA rasterize edges w/o supersampling
static void nk_tt__rasterize_sorted_edges(nk_tt__bitmap *result, nk_tt__edge *e, int n, int vsubsample, int off_x, int off_y, void *userdata)
{
   nk_tt__hheap hh = { 0, 0, 0 };
   nk_tt__active_edge *active = NULL;
   int y,j=0, i;
   float scanline_data[129], *scanline, *scanline2;

   NK_TT__NOTUSED(vsubsample);

   if (result->w > 64)
      scanline = (float *) NK_TT_malloc((result->w*2+1) * sizeof(float), userdata);
   else
      scanline = scanline_data;

   scanline2 = scanline + result->w;

   y = off_y;
   e[n].y0 = (float) (off_y + result->h) + 1;

   while (j < result->h) {
      // find center of pixel for this scanline
      float scan_y_top    = y + 0.0f;
      float scan_y_bottom = y + 1.0f;
      nk_tt__active_edge **step = &active;

      NK_TT_memset(scanline , 0, result->w*sizeof(scanline[0]));
      NK_TT_memset(scanline2, 0, (result->w+1)*sizeof(scanline[0]));

      // update all active edges;
      // remove all active edges that terminate before the top of this scanline
      while (*step) {
         nk_tt__active_edge * z = *step;
         if (z->ey <= scan_y_top) {
            *step = z->next; // delete from list
            NK_TT_assert(z->direction);
            z->direction = 0;
            nk_tt__hheap_free(&hh, z);
         } else {
            step = &((*step)->next); // advance through list
         }
      }

      // insert all edges that start before the bottom of this scanline
      while (e->y0 <= scan_y_bottom) {
         if (e->y0 != e->y1) {
            nk_tt__active_edge *z = nk_tt__new_active(&hh, e, off_x, scan_y_top, userdata);
            if (z != NULL) {
               if (j == 0 && off_y != 0) {
                  if (z->ey < scan_y_top) {
                     // this can happen due to subpixel positioning and some kind of fp rounding error i think
                     z->ey = scan_y_top;
                  }
               }
               NK_TT_assert(z->ey >= scan_y_top); // if we get really unlucky a tiny bit of an edge can be out of bounds
               // insert at front
               z->next = active;
               active = z;
            }
         }
         ++e;
      }

      // now process all active edges
      if (active)
         nk_tt__fill_active_edges_new(scanline, scanline2+1, result->w, active, scan_y_top);

      {
         float sum = 0;
         for (i=0; i < result->w; ++i) {
            float k;
            int m;
            sum += scanline2[i];
            k = scanline[i] + sum;
            k = (float) NK_TT_fabs(k)*255 + 0.5f;
            m = (int) k;
            if (m > 255) m = 255;
            result->pixels[j*result->stride + i] = (unsigned char) m;
         }
      }
      // advance all the edges
      step = &active;
      while (*step) {
         nk_tt__active_edge *z = *step;
         z->fx += z->fdx; // advance to position for current scanline
         step = &((*step)->next); // advance through list
      }

      ++y;
      ++j;
   }

   nk_tt__hheap_cleanup(&hh, userdata);

   if (scanline != scanline_data)
      NK_TT_free(scanline, userdata);
}
#else
#error "Unrecognized value of NK_TT_RASTERIZER_VERSION"
#endif

#define NK_TT__COMPARE(a,b)  ((a)->y0 < (b)->y0)

static void nk_tt__sort_edges_ins_sort(nk_tt__edge *p, int n)
{
   int i,j;
   for (i=1; i < n; ++i) {
      nk_tt__edge t = p[i], *a = &t;
      j = i;
      while (j > 0) {
         nk_tt__edge *b = &p[j-1];
         int c = NK_TT__COMPARE(a,b);
         if (!c) break;
         p[j] = p[j-1];
         --j;
      }
      if (i != j)
         p[j] = t;
   }
}

static void nk_tt__sort_edges_quicksort(nk_tt__edge *p, int n)
{
   /* threshold for transitioning to insertion sort */
   while (n > 12) {
      nk_tt__edge t;
      int c01,c12,c,m,i,j;

      /* compute median of three */
      m = n >> 1;
      c01 = NK_TT__COMPARE(&p[0],&p[m]);
      c12 = NK_TT__COMPARE(&p[m],&p[n-1]);
      /* if 0 >= mid >= end, or 0 < mid < end, then use mid */
      if (c01 != c12) {
         /* otherwise, we'll need to swap something else to middle */
         int z;
         c = NK_TT__COMPARE(&p[0],&p[n-1]);
         /* 0>mid && mid<n:  0>n => n; 0<n => 0 */
         /* 0<mid && mid>n:  0>n => 0; 0<n => n */
         z = (c == c12) ? 0 : n-1;
         t = p[z];
         p[z] = p[m];
         p[m] = t;
      }
      /* now p[m] is the median-of-three */
      /* swap it to the beginning so it won't move around */
      t = p[0];
      p[0] = p[m];
      p[m] = t;

      /* partition loop */
      i=1;
      j=n-1;
      for(;;) {
         /* handling of equality is crucial here */
         /* for sentinels & efficiency with duplicates */
         for (;;++i) {
            if (!NK_TT__COMPARE(&p[i], &p[0])) break;
         }
         for (;;--j) {
            if (!NK_TT__COMPARE(&p[0], &p[j])) break;
         }
         /* make sure we haven't crossed */
         if (i >= j) break;
         t = p[i];
         p[i] = p[j];
         p[j] = t;

         ++i;
         --j;
      }
      /* recurse on smaller side, iterate on larger */
      if (j < (n-i)) {
         nk_tt__sort_edges_quicksort(p,j);
         p = p+i;
         n = n-i;
      } else {
         nk_tt__sort_edges_quicksort(p+i, n-i);
         n = j;
      }
   }
}

static void nk_tt__sort_edges(nk_tt__edge *p, int n)
{
   nk_tt__sort_edges_quicksort(p, n);
   nk_tt__sort_edges_ins_sort(p, n);
}

typedef struct
{
   float x,y;
} nk_tt__point;

static void nk_tt__rasterize(nk_tt__bitmap *result, nk_tt__point *pts, int *wcount, int windings, float scale_x, float scale_y, float shift_x, float shift_y, int off_x, int off_y, int invert, void *userdata)
{
   float y_scale_inv = invert ? -scale_y : scale_y;
   nk_tt__edge *e;
   int n,i,j,k,m;
#if NK_TT_RASTERIZER_VERSION == 1
   int vsubsample = result->h < 8 ? 15 : 5;
#elif NK_TT_RASTERIZER_VERSION == 2
   int vsubsample = 1;
#else
   #error "Unrecognized value of NK_TT_RASTERIZER_VERSION"
#endif
   // vsubsample should divide 255 evenly; otherwise we won't reach full opacity

   // now we have to blow out the windings into explicit edge lists
   n = 0;
   for (i=0; i < windings; ++i)
      n += wcount[i];

   e = (nk_tt__edge *) NK_TT_malloc(sizeof(*e) * (n+1), userdata); // add an extra one as a sentinel
   if (e == 0) return;
   n = 0;

   m=0;
   for (i=0; i < windings; ++i) {
      nk_tt__point *p = pts + m;
      m += wcount[i];
      j = wcount[i]-1;
      for (k=0; k < wcount[i]; j=k++) {
         int a=k,b=j;
         // skip the edge if horizontal
         if (p[j].y == p[k].y)
            continue;
         // add edge from j to k to the list
         e[n].invert = 0;
         if (invert ? p[j].y > p[k].y : p[j].y < p[k].y) {
            e[n].invert = 1;
            a=j,b=k;
         }
         e[n].x0 = p[a].x * scale_x + shift_x;
         e[n].y0 = (p[a].y * y_scale_inv + shift_y) * vsubsample;
         e[n].x1 = p[b].x * scale_x + shift_x;
         e[n].y1 = (p[b].y * y_scale_inv + shift_y) * vsubsample;
         ++n;
      }
   }

   // now sort the edges by their highest point (should snap to integer, and then by x)
   //NK_TT_sort(e, n, sizeof(e[0]), nk_tt__edge_compare);
   nk_tt__sort_edges(e, n);

   // now, traverse the scanlines and find the intersections on each scanline, use xor winding rule
   nk_tt__rasterize_sorted_edges(result, e, n, vsubsample, off_x, off_y, userdata);

   NK_TT_free(e, userdata);
}

static void nk_tt__add_point(nk_tt__point *points, int n, float x, float y)
{
   if (!points) return; // during first pass, it's unallocated
   points[n].x = x;
   points[n].y = y;
}

// tessellate until threshold p is happy... @TODO warped to compensate for non-linear stretching
static int nk_tt__tesselate_curve(nk_tt__point *points, int *num_points, float x0, float y0, float x1, float y1, float x2, float y2, float objspace_flatness_squared, int n)
{
   // midpoint
   float mx = (x0 + 2*x1 + x2)/4;
   float my = (y0 + 2*y1 + y2)/4;
   // versus directly drawn line
   float dx = (x0+x2)/2 - mx;
   float dy = (y0+y2)/2 - my;
   if (n > 16) // 65536 segments on one curve better be enough!
      return 1;
   if (dx*dx+dy*dy > objspace_flatness_squared) { // half-pixel error allowed... need to be smaller if AA
      nk_tt__tesselate_curve(points, num_points, x0,y0, (x0+x1)/2.0f,(y0+y1)/2.0f, mx,my, objspace_flatness_squared,n+1);
      nk_tt__tesselate_curve(points, num_points, mx,my, (x1+x2)/2.0f,(y1+y2)/2.0f, x2,y2, objspace_flatness_squared,n+1);
   } else {
      nk_tt__add_point(points, *num_points,x2,y2);
      *num_points = *num_points+1;
   }
   return 1;
}

static void nk_tt__tesselate_cubic(nk_tt__point *points, int *num_points, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float objspace_flatness_squared, int n)
{
   // @TODO this "flatness" calculation is just made-up nonsense that seems to work well enough
   float dx0 = x1-x0;
   float dy0 = y1-y0;
   float dx1 = x2-x1;
   float dy1 = y2-y1;
   float dx2 = x3-x2;
   float dy2 = y3-y2;
   float dx = x3-x0;
   float dy = y3-y0;
   float longlen = (float) (NK_TT_sqrt(dx0*dx0+dy0*dy0)+NK_TT_sqrt(dx1*dx1+dy1*dy1)+NK_TT_sqrt(dx2*dx2+dy2*dy2));
   float shortlen = (float) NK_TT_sqrt(dx*dx+dy*dy);
   float flatness_squared = longlen*longlen-shortlen*shortlen;

   if (n > 16) // 65536 segments on one curve better be enough!
      return;

   if (flatness_squared > objspace_flatness_squared) {
      float x01 = (x0+x1)/2;
      float y01 = (y0+y1)/2;
      float x12 = (x1+x2)/2;
      float y12 = (y1+y2)/2;
      float x23 = (x2+x3)/2;
      float y23 = (y2+y3)/2;

      float xa = (x01+x12)/2;
      float ya = (y01+y12)/2;
      float xb = (x12+x23)/2;
      float yb = (y12+y23)/2;

      float mx = (xa+xb)/2;
      float my = (ya+yb)/2;

      nk_tt__tesselate_cubic(points, num_points, x0,y0, x01,y01, xa,ya, mx,my, objspace_flatness_squared,n+1);
      nk_tt__tesselate_cubic(points, num_points, mx,my, xb,yb, x23,y23, x3,y3, objspace_flatness_squared,n+1);
   } else {
      nk_tt__add_point(points, *num_points,x3,y3);
      *num_points = *num_points+1;
   }
}

// returns number of contours
static nk_tt__point *nk_tt_FlattenCurves(nk_tt_vertex *vertices, int num_verts, float objspace_flatness, int **contour_lengths, int *num_contours, void *userdata)
{
   nk_tt__point *points=0;
   int num_points=0;

   float objspace_flatness_squared = objspace_flatness * objspace_flatness;
   int i,n=0,start=0, pass;

   // count how many "moves" there are to get the contour count
   for (i=0; i < num_verts; ++i)
      if (vertices[i].type == NK_TT_vmove)
         ++n;

   *num_contours = n;
   if (n == 0) return 0;

   *contour_lengths = (int *) NK_TT_malloc(sizeof(**contour_lengths) * n, userdata);

   if (*contour_lengths == 0) {
      *num_contours = 0;
      return 0;
   }

   // make two passes through the points so we don't need to realloc
   for (pass=0; pass < 2; ++pass) {
      float x=0,y=0;
      if (pass == 1) {
         points = (nk_tt__point *) NK_TT_malloc(num_points * sizeof(points[0]), userdata);
         if (points == NULL) goto error;
      }
      num_points = 0;
      n= -1;
      for (i=0; i < num_verts; ++i) {
         switch (vertices[i].type) {
            case NK_TT_vmove:
               // start the next contour
               if (n >= 0)
                  (*contour_lengths)[n] = num_points - start;
               ++n;
               start = num_points;

               x = vertices[i].x, y = vertices[i].y;
               nk_tt__add_point(points, num_points++, x,y);
               break;
            case NK_TT_vline:
               x = vertices[i].x, y = vertices[i].y;
               nk_tt__add_point(points, num_points++, x, y);
               break;
            case NK_TT_vcurve:
               nk_tt__tesselate_curve(points, &num_points, x,y,
                                        vertices[i].cx, vertices[i].cy,
                                        vertices[i].x,  vertices[i].y,
                                        objspace_flatness_squared, 0);
               x = vertices[i].x, y = vertices[i].y;
               break;
            case NK_TT_vcubic:
               nk_tt__tesselate_cubic(points, &num_points, x,y,
                                        vertices[i].cx, vertices[i].cy,
                                        vertices[i].cx1, vertices[i].cy1,
                                        vertices[i].x,  vertices[i].y,
                                        objspace_flatness_squared, 0);
               x = vertices[i].x, y = vertices[i].y;
               break;
         }
      }
      (*contour_lengths)[n] = num_points - start;
   }

   return points;
error:
   NK_TT_free(points, userdata);
   NK_TT_free(*contour_lengths, userdata);
   *contour_lengths = 0;
   *num_contours = 0;
   return NULL;
}

NK_TT_DEF void nk_tt_Rasterize(nk_tt__bitmap *result, float flatness_in_pixels, nk_tt_vertex *vertices, int num_verts, float scale_x, float scale_y, float shift_x, float shift_y, int x_off, int y_off, int invert, void *userdata)
{
   float scale            = scale_x > scale_y ? scale_y : scale_x;
   int winding_count      = 0;
   int *winding_lengths   = NULL;
   nk_tt__point *windings = nk_tt_FlattenCurves(vertices, num_verts, flatness_in_pixels / scale, &winding_lengths, &winding_count, userdata);
   if (windings) {
      nk_tt__rasterize(result, windings, winding_lengths, winding_count, scale_x, scale_y, shift_x, shift_y, x_off, y_off, invert, userdata);
      NK_TT_free(winding_lengths, userdata);
      NK_TT_free(windings, userdata);
   }
}

NK_TT_DEF void nk_tt_FreeBitmap(unsigned char *bitmap, void *userdata)
{
   NK_TT_free(bitmap, userdata);
}

NK_TT_DEF unsigned char *nk_tt_GetGlyphBitmapSubpixel(const nk_tt_fontinfo *info, float scale_x, float scale_y, float shift_x, float shift_y, int glyph, int *width, int *height, int *xoff, int *yoff)
{
   int ix0,iy0,ix1,iy1;
   nk_tt__bitmap gbm;
   nk_tt_vertex *vertices;   
   int num_verts = nk_tt_GetGlyphShape(info, glyph, &vertices);

   if (scale_x == 0) scale_x = scale_y;
   if (scale_y == 0) {
      if (scale_x == 0) {
         NK_TT_free(vertices, info->userdata);
         return NULL;
      }
      scale_y = scale_x;
   }

   nk_tt_GetGlyphBitmapBoxSubpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, &ix0,&iy0,&ix1,&iy1);

   // now we get the size
   gbm.w = (ix1 - ix0);
   gbm.h = (iy1 - iy0);
   gbm.pixels = NULL; // in case we error

   if (width ) *width  = gbm.w;
   if (height) *height = gbm.h;
   if (xoff  ) *xoff   = ix0;
   if (yoff  ) *yoff   = iy0;
   
   if (gbm.w && gbm.h) {
      gbm.pixels = (unsigned char *) NK_TT_malloc(gbm.w * gbm.h, info->userdata);
      if (gbm.pixels) {
         gbm.stride = gbm.w;

         nk_tt_Rasterize(&gbm, 0.35f, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0, iy0, 1, info->userdata);
      }
   }
   NK_TT_free(vertices, info->userdata);
   return gbm.pixels;
}   

NK_TT_DEF unsigned char *nk_tt_GetGlyphBitmap(const nk_tt_fontinfo *info, float scale_x, float scale_y, int glyph, int *width, int *height, int *xoff, int *yoff)
{
   return nk_tt_GetGlyphBitmapSubpixel(info, scale_x, scale_y, 0.0f, 0.0f, glyph, width, height, xoff, yoff);
}

NK_TT_DEF void nk_tt_MakeGlyphBitmapSubpixel(const nk_tt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph)
{
   int ix0,iy0;
   nk_tt_vertex *vertices;
   int num_verts = nk_tt_GetGlyphShape(info, glyph, &vertices);
   nk_tt__bitmap gbm;   

   nk_tt_GetGlyphBitmapBoxSubpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, &ix0,&iy0,0,0);
   gbm.pixels = output;
   gbm.w = out_w;
   gbm.h = out_h;
   gbm.stride = out_stride;

   if (gbm.w && gbm.h)
      nk_tt_Rasterize(&gbm, 0.35f, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0,iy0, 1, info->userdata);

   NK_TT_free(vertices, info->userdata);
}

NK_TT_DEF void nk_tt_MakeGlyphBitmap(const nk_tt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int glyph)
{
   nk_tt_MakeGlyphBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, 0.0f,0.0f, glyph);
}

NK_TT_DEF unsigned char *nk_tt_GetCodepointBitmapSubpixel(const nk_tt_fontinfo *info, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint, int *width, int *height, int *xoff, int *yoff)
{
   return nk_tt_GetGlyphBitmapSubpixel(info, scale_x, scale_y,shift_x,shift_y, nk_tt_FindGlyphIndex(info,codepoint), width,height,xoff,yoff);
}   

NK_TT_DEF void nk_tt_MakeCodepointBitmapSubpixelPrefilter(const nk_tt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int oversample_x, int oversample_y, float *sub_x, float *sub_y, int codepoint)
{
   nk_tt_MakeGlyphBitmapSubpixelPrefilter(info, output, out_w, out_h, out_stride, scale_x, scale_y, shift_x, shift_y, oversample_x, oversample_y, sub_x, sub_y, nk_tt_FindGlyphIndex(info,codepoint));
}

NK_TT_DEF void nk_tt_MakeCodepointBitmapSubpixel(const nk_tt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint)
{
   nk_tt_MakeGlyphBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, shift_x, shift_y, nk_tt_FindGlyphIndex(info,codepoint));
}

NK_TT_DEF unsigned char *nk_tt_GetCodepointBitmap(const nk_tt_fontinfo *info, float scale_x, float scale_y, int codepoint, int *width, int *height, int *xoff, int *yoff)
{
   return nk_tt_GetCodepointBitmapSubpixel(info, scale_x, scale_y, 0.0f,0.0f, codepoint, width,height,xoff,yoff);
}   

NK_TT_DEF void nk_tt_MakeCodepointBitmap(const nk_tt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int codepoint)
{
   nk_tt_MakeCodepointBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, 0.0f,0.0f, codepoint);
}

//////////////////////////////////////////////////////////////////////////////
//
// bitmap baking
//
// This is SUPER-CRAPPY packing to keep source code small

static int nk_tt_BakeFontBitmap_internal(unsigned char *data, int offset,  // font location (use offset=0 for plain .ttf)
                                float pixel_height,                     // height of font in pixels
                                unsigned char *pixels, int pw, int ph,  // bitmap to be filled in
                                int first_char, int num_chars,          // characters to bake
                                nk_tt_bakedchar *chardata)
{
   float scale;
   int x,y,bottom_y, i;
   nk_tt_fontinfo f;
   f.userdata = NULL;
   if (!nk_tt_InitFont(&f, data, offset))
      return -1;
   NK_TT_memset(pixels, 0, pw*ph); // background of 0 around pixels
   x=y=1;
   bottom_y = 1;

   scale = nk_tt_ScaleForPixelHeight(&f, pixel_height);

   for (i=0; i < num_chars; ++i) {
      int advance, lsb, x0,y0,x1,y1,gw,gh;
      int g = nk_tt_FindGlyphIndex(&f, first_char + i);
      nk_tt_GetGlyphHMetrics(&f, g, &advance, &lsb);
      nk_tt_GetGlyphBitmapBox(&f, g, scale,scale, &x0,&y0,&x1,&y1);
      gw = x1-x0;
      gh = y1-y0;
      if (x + gw + 1 >= pw)
         y = bottom_y, x = 1; // advance to next row
      if (y + gh + 1 >= ph) // check if it fits vertically AFTER potentially moving to next row
         return -i;
      NK_TT_assert(x+gw < pw);
      NK_TT_assert(y+gh < ph);
      nk_tt_MakeGlyphBitmap(&f, pixels+x+y*pw, gw,gh,pw, scale,scale, g);
      chardata[i].x0 = (nk_tt_int16) x;
      chardata[i].y0 = (nk_tt_int16) y;
      chardata[i].x1 = (nk_tt_int16) (x + gw);
      chardata[i].y1 = (nk_tt_int16) (y + gh);
      chardata[i].xadvance = scale * advance;
      chardata[i].xoff     = (float) x0;
      chardata[i].yoff     = (float) y0;
      x = x + gw + 1;
      if (y+gh+1 > bottom_y)
         bottom_y = y+gh+1;
   }
   return bottom_y;
}

NK_TT_DEF void nk_tt_GetBakedQuad(const nk_tt_bakedchar *chardata, int pw, int ph, int char_index, float *xpos, float *ypos, nk_tt_aligned_quad *q, int opengl_fillrule)
{
   float d3d_bias = opengl_fillrule ? 0 : -0.5f;
   float ipw = 1.0f / pw, iph = 1.0f / ph;
   const nk_tt_bakedchar *b = chardata + char_index;
   int round_x = NK_TT_ifloor((*xpos + b->xoff) + 0.5f);
   int round_y = NK_TT_ifloor((*ypos + b->yoff) + 0.5f);

   q->x0 = round_x + d3d_bias;
   q->y0 = round_y + d3d_bias;
   q->x1 = round_x + b->x1 - b->x0 + d3d_bias;
   q->y1 = round_y + b->y1 - b->y0 + d3d_bias;

   q->s0 = b->x0 * ipw;
   q->t0 = b->y0 * iph;
   q->s1 = b->x1 * ipw;
   q->t1 = b->y1 * iph;

   *xpos += b->xadvance;
}

//////////////////////////////////////////////////////////////////////////////
//
// rectangle packing replacement routines if you don't have stb_rect_pack.h
//

#ifndef STB_RECT_PACK_VERSION

typedef int nk_rp_coord;

////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
//                                                                                //
// COMPILER WARNING ?!?!?                                                         //
//                                                                                //
//                                                                                //
// if you get a compile warning due to these symbols being defined more than      //
// once, move #include "stb_rect_pack.h" before #include "stb_truetype.h"         //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
   int width,height;
   int x,y,bottom_y;
} nk_rp_context;

typedef struct
{
   unsigned char x;
} nk_rp_node;

struct nk_rp_rect
{
   nk_rp_coord x,y;
   int id,w,h,was_packed;
};

static void nk_rp_init_target(nk_rp_context *con, int pw, int ph, nk_rp_node *nodes, int num_nodes)
{
   con->width  = pw;
   con->height = ph;
   con->x = 0;
   con->y = 0;
   con->bottom_y = 0;
   NK_TT__NOTUSED(nodes);
   NK_TT__NOTUSED(num_nodes);   
}

static void nk_rp_pack_rects(nk_rp_context *con, nk_rp_rect *rects, int num_rects)
{
   int i;
   for (i=0; i < num_rects; ++i) {
      if (con->x + rects[i].w > con->width) {
         con->x = 0;
         con->y = con->bottom_y;
      }
      if (con->y + rects[i].h > con->height)
         break;
      rects[i].x = con->x;
      rects[i].y = con->y;
      rects[i].was_packed = 1;
      con->x += rects[i].w;
      if (con->y + rects[i].h > con->bottom_y)
         con->bottom_y = con->y + rects[i].h;
   }
   for (   ; i < num_rects; ++i)
      rects[i].was_packed = 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// bitmap baking
//
// This is SUPER-AWESOME (tm Ryan Gordon) packing using stb_rect_pack.h. If
// stb_rect_pack.h isn't available, it uses the BakeFontBitmap strategy.

NK_TT_DEF int nk_tt_PackBegin(nk_tt_pack_context *spc, unsigned char *pixels, int pw, int ph, int stride_in_bytes, int padding, void *alloc_context)
{
   nk_rp_context *context = (nk_rp_context *) NK_TT_malloc(sizeof(*context)            ,alloc_context);
   int            num_nodes = pw - padding;
   nk_rp_node    *nodes   = (nk_rp_node    *) NK_TT_malloc(sizeof(*nodes  ) * num_nodes,alloc_context);

   if (context == NULL || nodes == NULL) {
      if (context != NULL) NK_TT_free(context, alloc_context);
      if (nodes   != NULL) NK_TT_free(nodes  , alloc_context);
      return 0;
   }

   spc->user_allocator_context = alloc_context;
   spc->width = pw;
   spc->height = ph;
   spc->pixels = pixels;
   spc->pack_info = context;
   spc->nodes = nodes;
   spc->padding = padding;
   spc->stride_in_bytes = stride_in_bytes != 0 ? stride_in_bytes : pw;
   spc->h_oversample = 1;
   spc->v_oversample = 1;
   spc->skip_missing = 0;

   nk_rp_init_target(context, pw-padding, ph-padding, nodes, num_nodes);

   if (pixels)
      NK_TT_memset(pixels, 0, pw*ph); // background of 0 around pixels

   return 1;
}

NK_TT_DEF void nk_tt_PackEnd  (nk_tt_pack_context *spc)
{
   NK_TT_free(spc->nodes    , spc->user_allocator_context);
   NK_TT_free(spc->pack_info, spc->user_allocator_context);
}

NK_TT_DEF void nk_tt_PackSetOversampling(nk_tt_pack_context *spc, unsigned int h_oversample, unsigned int v_oversample)
{
   NK_TT_assert(h_oversample <= NK_TT_MAX_OVERSAMPLE);
   NK_TT_assert(v_oversample <= NK_TT_MAX_OVERSAMPLE);
   if (h_oversample <= NK_TT_MAX_OVERSAMPLE)
      spc->h_oversample = h_oversample;
   if (v_oversample <= NK_TT_MAX_OVERSAMPLE)
      spc->v_oversample = v_oversample;
}

NK_TT_DEF void nk_tt_PackSetSkipMissingCodepoints(nk_tt_pack_context *spc, int skip)
{
   spc->skip_missing = skip;
}

#define NK_TT__OVER_MASK  (NK_TT_MAX_OVERSAMPLE-1)

static void nk_tt__h_prefilter(unsigned char *pixels, int w, int h, int stride_in_bytes, unsigned int kernel_width)
{
   unsigned char buffer[NK_TT_MAX_OVERSAMPLE];
   int safe_w = w - kernel_width;
   int j;
   NK_TT_memset(buffer, 0, NK_TT_MAX_OVERSAMPLE); // suppress bogus warning from VS2013 -analyze
   for (j=0; j < h; ++j) {
      int i;
      unsigned int total;
      NK_TT_memset(buffer, 0, kernel_width);

      total = 0;

      // make kernel_width a constant in common cases so compiler can optimize out the divide
      switch (kernel_width) {
         case 2:
            for (i=0; i <= safe_w; ++i) {
               total += pixels[i] - buffer[i & NK_TT__OVER_MASK];
               buffer[(i+kernel_width) & NK_TT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / 2);
            }
            break;
         case 3:
            for (i=0; i <= safe_w; ++i) {
               total += pixels[i] - buffer[i & NK_TT__OVER_MASK];
               buffer[(i+kernel_width) & NK_TT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / 3);
            }
            break;
         case 4:
            for (i=0; i <= safe_w; ++i) {
               total += pixels[i] - buffer[i & NK_TT__OVER_MASK];
               buffer[(i+kernel_width) & NK_TT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / 4);
            }
            break;
         case 5:
            for (i=0; i <= safe_w; ++i) {
               total += pixels[i] - buffer[i & NK_TT__OVER_MASK];
               buffer[(i+kernel_width) & NK_TT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / 5);
            }
            break;
         default:
            for (i=0; i <= safe_w; ++i) {
               total += pixels[i] - buffer[i & NK_TT__OVER_MASK];
               buffer[(i+kernel_width) & NK_TT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / kernel_width);
            }
            break;
      }

      for (; i < w; ++i) {
         NK_TT_assert(pixels[i] == 0);
         total -= buffer[i & NK_TT__OVER_MASK];
         pixels[i] = (unsigned char) (total / kernel_width);
      }

      pixels += stride_in_bytes;
   }
}

static void nk_tt__v_prefilter(unsigned char *pixels, int w, int h, int stride_in_bytes, unsigned int kernel_width)
{
   unsigned char buffer[NK_TT_MAX_OVERSAMPLE];
   int safe_h = h - kernel_width;
   int j;
   NK_TT_memset(buffer, 0, NK_TT_MAX_OVERSAMPLE); // suppress bogus warning from VS2013 -analyze
   for (j=0; j < w; ++j) {
      int i;
      unsigned int total;
      NK_TT_memset(buffer, 0, kernel_width);

      total = 0;

      // make kernel_width a constant in common cases so compiler can optimize out the divide
      switch (kernel_width) {
         case 2:
            for (i=0; i <= safe_h; ++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & NK_TT__OVER_MASK];
               buffer[(i+kernel_width) & NK_TT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / 2);
            }
            break;
         case 3:
            for (i=0; i <= safe_h; ++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & NK_TT__OVER_MASK];
               buffer[(i+kernel_width) & NK_TT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / 3);
            }
            break;
         case 4:
            for (i=0; i <= safe_h; ++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & NK_TT__OVER_MASK];
               buffer[(i+kernel_width) & NK_TT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / 4);
            }
            break;
         case 5:
            for (i=0; i <= safe_h; ++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & NK_TT__OVER_MASK];
               buffer[(i+kernel_width) & NK_TT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / 5);
            }
            break;
         default:
            for (i=0; i <= safe_h; ++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & NK_TT__OVER_MASK];
               buffer[(i+kernel_width) & NK_TT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / kernel_width);
            }
            break;
      }

      for (; i < h; ++i) {
         NK_TT_assert(pixels[i*stride_in_bytes] == 0);
         total -= buffer[i & NK_TT__OVER_MASK];
         pixels[i*stride_in_bytes] = (unsigned char) (total / kernel_width);
      }

      pixels += 1;
   }
}

static float nk_tt__oversample_shift(int oversample)
{
   if (!oversample)
      return 0.0f;

   // The prefilter is a box filter of width "oversample",
   // which shifts phase by (oversample - 1)/2 pixels in
   // oversampled space. We want to shift in the opposite
   // direction to counter this.
   return (float)-(oversample - 1) / (2.0f * (float)oversample);
}

// rects array must be big enough to accommodate all characters in the given ranges
NK_TT_DEF int nk_tt_PackFontRangesGatherRects(nk_tt_pack_context *spc, const nk_tt_fontinfo *info, nk_tt_pack_range *ranges, int num_ranges, nk_rp_rect *rects)
{
   int i,j,k;
   int missing_glyph_added = 0;

   k=0;
   for (i=0; i < num_ranges; ++i) {
      float fh = ranges[i].font_size;
      float scale = fh > 0 ? nk_tt_ScaleForPixelHeight(info, fh) : nk_tt_ScaleForMappingEmToPixels(info, -fh);
      ranges[i].h_oversample = (unsigned char) spc->h_oversample;
      ranges[i].v_oversample = (unsigned char) spc->v_oversample;
      for (j=0; j < ranges[i].num_chars; ++j) {
         int x0,y0,x1,y1;
         int codepoint = ranges[i].array_of_unicode_codepoints == NULL ? ranges[i].first_unicode_codepoint_in_range + j : ranges[i].array_of_unicode_codepoints[j];
         int glyph = nk_tt_FindGlyphIndex(info, codepoint);
         if (glyph == 0 && (spc->skip_missing || missing_glyph_added)) {
            rects[k].w = rects[k].h = 0;
         } else {
            nk_tt_GetGlyphBitmapBoxSubpixel(info,glyph,
                                            scale * spc->h_oversample,
                                            scale * spc->v_oversample,
                                            0,0,
                                            &x0,&y0,&x1,&y1);
            rects[k].w = (nk_rp_coord) (x1-x0 + spc->padding + spc->h_oversample-1);
            rects[k].h = (nk_rp_coord) (y1-y0 + spc->padding + spc->v_oversample-1);
            if (glyph == 0)
               missing_glyph_added = 1;
         }
         ++k;
      }
   }

   return k;
}

NK_TT_DEF void nk_tt_MakeGlyphBitmapSubpixelPrefilter(const nk_tt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int prefilter_x, int prefilter_y, float *sub_x, float *sub_y, int glyph)
{
   nk_tt_MakeGlyphBitmapSubpixel(info,
                                 output,
                                 out_w - (prefilter_x - 1),
                                 out_h - (prefilter_y - 1),
                                 out_stride,
                                 scale_x,
                                 scale_y,
                                 shift_x,
                                 shift_y,
                                 glyph);

   if (prefilter_x > 1)
      nk_tt__h_prefilter(output, out_w, out_h, out_stride, prefilter_x);

   if (prefilter_y > 1)
      nk_tt__v_prefilter(output, out_w, out_h, out_stride, prefilter_y);

   *sub_x = nk_tt__oversample_shift(prefilter_x);
   *sub_y = nk_tt__oversample_shift(prefilter_y);
}

// rects array must be big enough to accommodate all characters in the given ranges
NK_TT_DEF int nk_tt_PackFontRangesRenderIntoRects(nk_tt_pack_context *spc, const nk_tt_fontinfo *info, nk_tt_pack_range *ranges, int num_ranges, nk_rp_rect *rects)
{
   int i,j,k, missing_glyph = -1, return_value = 1;

   // save current values
   int old_h_over = spc->h_oversample;
   int old_v_over = spc->v_oversample;

   k = 0;
   for (i=0; i < num_ranges; ++i) {
      float fh = ranges[i].font_size;
      float scale = fh > 0 ? nk_tt_ScaleForPixelHeight(info, fh) : nk_tt_ScaleForMappingEmToPixels(info, -fh);
      float recip_h,recip_v,sub_x,sub_y;
      spc->h_oversample = ranges[i].h_oversample;
      spc->v_oversample = ranges[i].v_oversample;
      recip_h = 1.0f / spc->h_oversample;
      recip_v = 1.0f / spc->v_oversample;
      sub_x = nk_tt__oversample_shift(spc->h_oversample);
      sub_y = nk_tt__oversample_shift(spc->v_oversample);
      for (j=0; j < ranges[i].num_chars; ++j) {
         nk_rp_rect *r = &rects[k];
         if (r->was_packed && r->w != 0 && r->h != 0) {
            nk_tt_packedchar *bc = &ranges[i].chardata_for_range[j];
            int advance, lsb, x0,y0,x1,y1;
            int codepoint = ranges[i].array_of_unicode_codepoints == NULL ? ranges[i].first_unicode_codepoint_in_range + j : ranges[i].array_of_unicode_codepoints[j];
            int glyph = nk_tt_FindGlyphIndex(info, codepoint);
            nk_rp_coord pad = (nk_rp_coord) spc->padding;

            // pad on left and top
            r->x += pad;
            r->y += pad;
            r->w -= pad;
            r->h -= pad;
            nk_tt_GetGlyphHMetrics(info, glyph, &advance, &lsb);
            nk_tt_GetGlyphBitmapBox(info, glyph,
                                    scale * spc->h_oversample,
                                    scale * spc->v_oversample,
                                    &x0,&y0,&x1,&y1);
            nk_tt_MakeGlyphBitmapSubpixel(info,
                                          spc->pixels + r->x + r->y*spc->stride_in_bytes,
                                          r->w - spc->h_oversample+1,
                                          r->h - spc->v_oversample+1,
                                          spc->stride_in_bytes,
                                          scale * spc->h_oversample,
                                          scale * spc->v_oversample,
                                          0,0,
                                          glyph);

            if (spc->h_oversample > 1)
               nk_tt__h_prefilter(spc->pixels + r->x + r->y*spc->stride_in_bytes,
                                  r->w, r->h, spc->stride_in_bytes,
                                  spc->h_oversample);

            if (spc->v_oversample > 1)
               nk_tt__v_prefilter(spc->pixels + r->x + r->y*spc->stride_in_bytes,
                                  r->w, r->h, spc->stride_in_bytes,
                                  spc->v_oversample);

            bc->x0       = (nk_tt_int16)  r->x;
            bc->y0       = (nk_tt_int16)  r->y;
            bc->x1       = (nk_tt_int16) (r->x + r->w);
            bc->y1       = (nk_tt_int16) (r->y + r->h);
            bc->xadvance =                scale * advance;
            bc->xoff     =       (float)  x0 * recip_h + sub_x;
            bc->yoff     =       (float)  y0 * recip_v + sub_y;
            bc->xoff2    =                (x0 + r->w) * recip_h + sub_x;
            bc->yoff2    =                (y0 + r->h) * recip_v + sub_y;

            if (glyph == 0)
               missing_glyph = j;
         } else if (spc->skip_missing) {
            return_value = 0;
         } else if (r->was_packed && r->w == 0 && r->h == 0 && missing_glyph >= 0) {
            ranges[i].chardata_for_range[j] = ranges[i].chardata_for_range[missing_glyph];
         } else {
            return_value = 0; // if any fail, report failure
         }

         ++k;
      }
   }

   // restore original values
   spc->h_oversample = old_h_over;
   spc->v_oversample = old_v_over;

   return return_value;
}

NK_TT_DEF void nk_tt_PackFontRangesPackRects(nk_tt_pack_context *spc, nk_rp_rect *rects, int num_rects)
{
   nk_rp_pack_rects((nk_rp_context *) spc->pack_info, rects, num_rects);
}

NK_TT_DEF int nk_tt_PackFontRanges(nk_tt_pack_context *spc, const unsigned char *fontdata, int font_index, nk_tt_pack_range *ranges, int num_ranges)
{
   nk_tt_fontinfo info;
   int i,j,n, return_value = 1;
   //nk_rp_context *context = (nk_rp_context *) spc->pack_info;
   nk_rp_rect    *rects;

   // flag all characters as NOT packed
   for (i=0; i < num_ranges; ++i)
      for (j=0; j < ranges[i].num_chars; ++j)
         ranges[i].chardata_for_range[j].x0 =
         ranges[i].chardata_for_range[j].y0 =
         ranges[i].chardata_for_range[j].x1 =
         ranges[i].chardata_for_range[j].y1 = 0;

   n = 0;
   for (i=0; i < num_ranges; ++i)
      n += ranges[i].num_chars;
         
   rects = (nk_rp_rect *) NK_TT_malloc(sizeof(*rects) * n, spc->user_allocator_context);
   if (rects == NULL)
      return 0;

   info.userdata = spc->user_allocator_context;
   nk_tt_InitFont(&info, fontdata, nk_tt_GetFontOffsetForIndex(fontdata,font_index));

   n = nk_tt_PackFontRangesGatherRects(spc, &info, ranges, num_ranges, rects);

   nk_tt_PackFontRangesPackRects(spc, rects, n);
  
   return_value = nk_tt_PackFontRangesRenderIntoRects(spc, &info, ranges, num_ranges, rects);

   NK_TT_free(rects, spc->user_allocator_context);
   return return_value;
}

NK_TT_DEF int nk_tt_PackFontRange(nk_tt_pack_context *spc, const unsigned char *fontdata, int font_index, float font_size,
            int first_unicode_codepoint_in_range, int num_chars_in_range, nk_tt_packedchar *chardata_for_range)
{
   nk_tt_pack_range range;
   range.first_unicode_codepoint_in_range = first_unicode_codepoint_in_range;
   range.array_of_unicode_codepoints = NULL;
   range.num_chars                   = num_chars_in_range;
   range.chardata_for_range          = chardata_for_range;
   range.font_size                   = font_size;
   return nk_tt_PackFontRanges(spc, fontdata, font_index, &range, 1);
}

NK_TT_DEF void nk_tt_GetScaledFontVMetrics(const unsigned char *fontdata, int index, float size, float *ascent, float *descent, float *lineGap)
{
   int i_ascent, i_descent, i_lineGap;
   float scale;
   nk_tt_fontinfo info;
   nk_tt_InitFont(&info, fontdata, nk_tt_GetFontOffsetForIndex(fontdata, index));
   scale = size > 0 ? nk_tt_ScaleForPixelHeight(&info, size) : nk_tt_ScaleForMappingEmToPixels(&info, -size);
   nk_tt_GetFontVMetrics(&info, &i_ascent, &i_descent, &i_lineGap);
   *ascent  = (float) i_ascent  * scale;
   *descent = (float) i_descent * scale;
   *lineGap = (float) i_lineGap * scale;
}

NK_TT_DEF void nk_tt_GetPackedQuad(const nk_tt_packedchar *chardata, int pw, int ph, int char_index, float *xpos, float *ypos, nk_tt_aligned_quad *q, int align_to_integer)
{
   float ipw = 1.0f / pw, iph = 1.0f / ph;
   const nk_tt_packedchar *b = chardata + char_index;

   if (align_to_integer) {
      float x = (float) NK_TT_ifloor((*xpos + b->xoff) + 0.5f);
      float y = (float) NK_TT_ifloor((*ypos + b->yoff) + 0.5f);
      q->x0 = x;
      q->y0 = y;
      q->x1 = x + b->xoff2 - b->xoff;
      q->y1 = y + b->yoff2 - b->yoff;
   } else {
      q->x0 = *xpos + b->xoff;
      q->y0 = *ypos + b->yoff;
      q->x1 = *xpos + b->xoff2;
      q->y1 = *ypos + b->yoff2;
   }

   q->s0 = b->x0 * ipw;
   q->t0 = b->y0 * iph;
   q->s1 = b->x1 * ipw;
   q->t1 = b->y1 * iph;

   *xpos += b->xadvance;
}

//////////////////////////////////////////////////////////////////////////////
//
// sdf computation
//

#define NK_TT_min(a,b)  ((a) < (b) ? (a) : (b))
#define NK_TT_max(a,b)  ((a) < (b) ? (b) : (a))

static int nk_tt__ray_intersect_bezier(float orig[2], float ray[2], float q0[2], float q1[2], float q2[2], float hits[2][2])
{
   float q0perp = q0[1]*ray[0] - q0[0]*ray[1];
   float q1perp = q1[1]*ray[0] - q1[0]*ray[1];
   float q2perp = q2[1]*ray[0] - q2[0]*ray[1];
   float roperp = orig[1]*ray[0] - orig[0]*ray[1];

   float a = q0perp - 2*q1perp + q2perp;
   float b = q1perp - q0perp;
   float c = q0perp - roperp;

   float s0 = 0., s1 = 0.;
   int num_s = 0;

   if (a != 0.0) {
      float discr = b*b - a*c;
      if (discr > 0.0) {
         float rcpna = -1 / a;
         float d = (float) NK_TT_sqrt(discr);
         s0 = (b+d) * rcpna;
         s1 = (b-d) * rcpna;
         if (s0 >= 0.0 && s0 <= 1.0)
            num_s = 1;
         if (d > 0.0 && s1 >= 0.0 && s1 <= 1.0) {
            if (num_s == 0) s0 = s1;
            ++num_s;
         }
      }
   } else {
      // 2*b*s + c = 0
      // s = -c / (2*b)
      s0 = c / (-2 * b);
      if (s0 >= 0.0 && s0 <= 1.0)
         num_s = 1;
   }

   if (num_s == 0)
      return 0;
   else {
      float rcp_len2 = 1 / (ray[0]*ray[0] + ray[1]*ray[1]);
      float rayn_x = ray[0] * rcp_len2, rayn_y = ray[1] * rcp_len2;

      float q0d =   q0[0]*rayn_x +   q0[1]*rayn_y;
      float q1d =   q1[0]*rayn_x +   q1[1]*rayn_y;
      float q2d =   q2[0]*rayn_x +   q2[1]*rayn_y;
      float rod = orig[0]*rayn_x + orig[1]*rayn_y;

      float q10d = q1d - q0d;
      float q20d = q2d - q0d;
      float q0rd = q0d - rod;

      hits[0][0] = q0rd + s0*(2.0f - 2.0f*s0)*q10d + s0*s0*q20d;
      hits[0][1] = a*s0+b;

      if (num_s > 1) {
         hits[1][0] = q0rd + s1*(2.0f - 2.0f*s1)*q10d + s1*s1*q20d;
         hits[1][1] = a*s1+b;
         return 2;
      } else {
         return 1;
      }
   }
}

static int equal(float *a, float *b)
{
   return (a[0] == b[0] && a[1] == b[1]);
}

static int nk_tt__compute_crossings_x(float x, float y, int nverts, nk_tt_vertex *verts)
{
   int i;
   float orig[2], ray[2] = { 1, 0 };
   float y_frac;
   int winding = 0;

   orig[0] = x;
   orig[1] = y;

   // make sure y never passes through a vertex of the shape
   y_frac = (float) NK_TT_fmod(y, 1.0f);
   if (y_frac < 0.01f)
      y += 0.01f;
   else if (y_frac > 0.99f)
      y -= 0.01f;
   orig[1] = y;

   // test a ray from (-infinity,y) to (x,y)
   for (i=0; i < nverts; ++i) {
      if (verts[i].type == NK_TT_vline) {
         int x0 = (int) verts[i-1].x, y0 = (int) verts[i-1].y;
         int x1 = (int) verts[i  ].x, y1 = (int) verts[i  ].y;
         if (y > NK_TT_min(y0,y1) && y < NK_TT_max(y0,y1) && x > NK_TT_min(x0,x1)) {
            float x_inter = (y - y0) / (y1 - y0) * (x1-x0) + x0;
            if (x_inter < x)  
               winding += (y0 < y1) ? 1 : -1;
         }
      }
      if (verts[i].type == NK_TT_vcurve) {
         int x0 = (int) verts[i-1].x , y0 = (int) verts[i-1].y ;
         int x1 = (int) verts[i  ].cx, y1 = (int) verts[i  ].cy;
         int x2 = (int) verts[i  ].x , y2 = (int) verts[i  ].y ;
         int ax = NK_TT_min(x0,NK_TT_min(x1,x2)), ay = NK_TT_min(y0,NK_TT_min(y1,y2));
         int by = NK_TT_max(y0,NK_TT_max(y1,y2));
         if (y > ay && y < by && x > ax) {
            float q0[2],q1[2],q2[2];
            float hits[2][2];
            q0[0] = (float)x0;
            q0[1] = (float)y0;
            q1[0] = (float)x1;
            q1[1] = (float)y1;
            q2[0] = (float)x2;
            q2[1] = (float)y2;
            if (equal(q0,q1) || equal(q1,q2)) {
               x0 = (int)verts[i-1].x;
               y0 = (int)verts[i-1].y;
               x1 = (int)verts[i  ].x;
               y1 = (int)verts[i  ].y;
               if (y > NK_TT_min(y0,y1) && y < NK_TT_max(y0,y1) && x > NK_TT_min(x0,x1)) {
                  float x_inter = (y - y0) / (y1 - y0) * (x1-x0) + x0;
                  if (x_inter < x)  
                     winding += (y0 < y1) ? 1 : -1;
               }
            } else {
               int num_hits = nk_tt__ray_intersect_bezier(orig, ray, q0, q1, q2, hits);
               if (num_hits >= 1)
                  if (hits[0][0] < 0)
                     winding += (hits[0][1] < 0 ? -1 : 1);
               if (num_hits >= 2)
                  if (hits[1][0] < 0)
                     winding += (hits[1][1] < 0 ? -1 : 1);
            }
         } 
      }
   }
   return winding;
}

static float nk_tt__cuberoot( float x )
{
   if (x<0)
      return -(float) NK_TT_pow(-x,1.0f/3.0f);
   else
      return  (float) NK_TT_pow( x,1.0f/3.0f);
}

// x^3 + c*x^2 + b*x + a = 0
static int nk_tt__solve_cubic(float a, float b, float c, float* r)
{
	float s = -a / 3;
	float p = b - a*a / 3;
	float q = a * (2*a*a - 9*b) / 27 + c;
   float p3 = p*p*p;
	float d = q*q + 4*p3 / 27;
	if (d >= 0) {
		float z = (float) NK_TT_sqrt(d);
		float u = (-q + z) / 2;
		float v = (-q - z) / 2;
		u = nk_tt__cuberoot(u);
		v = nk_tt__cuberoot(v);
		r[0] = s + u + v;
		return 1;
	} else {
	   float u = (float) NK_TT_sqrt(-p/3);
	   float v = (float) NK_TT_acos(-NK_TT_sqrt(-27/p3) * q / 2) / 3; // p3 must be negative, since d is negative
	   float m = (float) NK_TT_cos(v);
      float n = (float) NK_TT_cos(v-3.141592/2)*1.732050808f;
	   r[0] = s + u * 2 * m;
	   r[1] = s - u * (m + n);
	   r[2] = s - u * (m - n);

      //NK_TT_assert( NK_TT_fabs(((r[0]+a)*r[0]+b)*r[0]+c) < 0.05f);  // these asserts may not be safe at all scales, though they're in bezier t parameter units so maybe?
      //NK_TT_assert( NK_TT_fabs(((r[1]+a)*r[1]+b)*r[1]+c) < 0.05f);
      //NK_TT_assert( NK_TT_fabs(((r[2]+a)*r[2]+b)*r[2]+c) < 0.05f);
   	return 3;
   }
}

NK_TT_DEF unsigned char * nk_tt_GetGlyphSDF(const nk_tt_fontinfo *info, float scale, int glyph, int padding, unsigned char onedge_value, float pixel_dist_scale, int *width, int *height, int *xoff, int *yoff)
{
   float scale_x = scale, scale_y = scale;
   int ix0,iy0,ix1,iy1;
   int w,h;
   unsigned char *data;

   if (scale == 0) return NULL;

   nk_tt_GetGlyphBitmapBoxSubpixel(info, glyph, scale, scale, 0.0f,0.0f, &ix0,&iy0,&ix1,&iy1);

   // if empty, return NULL
   if (ix0 == ix1 || iy0 == iy1)
      return NULL;

   ix0 -= padding;
   iy0 -= padding;
   ix1 += padding;
   iy1 += padding;

   w = (ix1 - ix0);
   h = (iy1 - iy0);

   if (width ) *width  = w;
   if (height) *height = h;
   if (xoff  ) *xoff   = ix0;
   if (yoff  ) *yoff   = iy0;

   // invert for y-downwards bitmaps
   scale_y = -scale_y;
      
   {
      int x,y,i,j;
      float *precompute;
      nk_tt_vertex *verts;
      int num_verts = nk_tt_GetGlyphShape(info, glyph, &verts);
      data = (unsigned char *) NK_TT_malloc(w * h, info->userdata);
      precompute = (float *) NK_TT_malloc(num_verts * sizeof(float), info->userdata);

      for (i=0,j=num_verts-1; i < num_verts; j=i++) {
         if (verts[i].type == NK_TT_vline) {
            float x0 = verts[i].x*scale_x, y0 = verts[i].y*scale_y;
            float x1 = verts[j].x*scale_x, y1 = verts[j].y*scale_y;
            float dist = (float) NK_TT_sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
            precompute[i] = (dist == 0) ? 0.0f : 1.0f / dist;
         } else if (verts[i].type == NK_TT_vcurve) {
            float x2 = verts[j].x *scale_x, y2 = verts[j].y *scale_y;
            float x1 = verts[i].cx*scale_x, y1 = verts[i].cy*scale_y;
            float x0 = verts[i].x *scale_x, y0 = verts[i].y *scale_y;
            float bx = x0 - 2*x1 + x2, by = y0 - 2*y1 + y2;
            float len2 = bx*bx + by*by;
            if (len2 != 0.0f)
               precompute[i] = 1.0f / (bx*bx + by*by);
            else
               precompute[i] = 0.0f;
         } else
            precompute[i] = 0.0f;
      }

      for (y=iy0; y < iy1; ++y) {
         for (x=ix0; x < ix1; ++x) {
            float val;
            float min_dist = 999999.0f;
            float sx = (float) x + 0.5f;
            float sy = (float) y + 0.5f;
            float x_gspace = (sx / scale_x);
            float y_gspace = (sy / scale_y);

            int winding = nk_tt__compute_crossings_x(x_gspace, y_gspace, num_verts, verts); // @OPTIMIZE: this could just be a rasterization, but needs to be line vs. non-tesselated curves so a new path

            for (i=0; i < num_verts; ++i) {
               float x0 = verts[i].x*scale_x, y0 = verts[i].y*scale_y;

               // check against every point here rather than inside line/curve primitives -- @TODO: wrong if multiple 'moves' in a row produce a garbage point, and given culling, probably more efficient to do within line/curve
               float dist2 = (x0-sx)*(x0-sx) + (y0-sy)*(y0-sy);
               if (dist2 < min_dist*min_dist)
                  min_dist = (float) NK_TT_sqrt(dist2);

               if (verts[i].type == NK_TT_vline) {
                  float x1 = verts[i-1].x*scale_x, y1 = verts[i-1].y*scale_y;

                  // coarse culling against bbox
                  //if (sx > NK_TT_min(x0,x1)-min_dist && sx < NK_TT_max(x0,x1)+min_dist &&
                  //    sy > NK_TT_min(y0,y1)-min_dist && sy < NK_TT_max(y0,y1)+min_dist)
                  float dist = (float) NK_TT_fabs((x1-x0)*(y0-sy) - (y1-y0)*(x0-sx)) * precompute[i];
                  NK_TT_assert(i != 0);
                  if (dist < min_dist) {
                     // check position along line
                     // x' = x0 + t*(x1-x0), y' = y0 + t*(y1-y0)
                     // minimize (x'-sx)*(x'-sx)+(y'-sy)*(y'-sy)
                     float dx = x1-x0, dy = y1-y0;
                     float px = x0-sx, py = y0-sy;
                     // minimize (px+t*dx)^2 + (py+t*dy)^2 = px*px + 2*px*dx*t + t^2*dx*dx + py*py + 2*py*dy*t + t^2*dy*dy
                     // derivative: 2*px*dx + 2*py*dy + (2*dx*dx+2*dy*dy)*t, set to 0 and solve
                     float t = -(px*dx + py*dy) / (dx*dx + dy*dy);
                     if (t >= 0.0f && t <= 1.0f)
                        min_dist = dist;
                  }
               } else if (verts[i].type == NK_TT_vcurve) {
                  float x2 = verts[i-1].x *scale_x, y2 = verts[i-1].y *scale_y;
                  float x1 = verts[i  ].cx*scale_x, y1 = verts[i  ].cy*scale_y;
                  float box_x0 = NK_TT_min(NK_TT_min(x0,x1),x2);
                  float box_y0 = NK_TT_min(NK_TT_min(y0,y1),y2);
                  float box_x1 = NK_TT_max(NK_TT_max(x0,x1),x2);
                  float box_y1 = NK_TT_max(NK_TT_max(y0,y1),y2);
                  // coarse culling against bbox to avoid computing cubic unnecessarily
                  if (sx > box_x0-min_dist && sx < box_x1+min_dist && sy > box_y0-min_dist && sy < box_y1+min_dist) {
                     int num=0;
                     float ax = x1-x0, ay = y1-y0;
                     float bx = x0 - 2*x1 + x2, by = y0 - 2*y1 + y2;
                     float mx = x0 - sx, my = y0 - sy;
                     float res[3],px,py,t,it;
                     float a_inv = precompute[i];
                     if (a_inv == 0.0) { // if a_inv is 0, it's 2nd degree so use quadratic formula
                        float a = 3*(ax*bx + ay*by);
                        float b = 2*(ax*ax + ay*ay) + (mx*bx+my*by);
                        float c = mx*ax+my*ay;
                        if (a == 0.0) { // if a is 0, it's linear
                           if (b != 0.0) {
                              res[num++] = -c/b;
                           }
                        } else {
                           float discriminant = b*b - 4*a*c;
                           if (discriminant < 0)
                              num = 0;
                           else {
                              float root = (float) NK_TT_sqrt(discriminant);
                              res[0] = (-b - root)/(2*a);
                              res[1] = (-b + root)/(2*a);
                              num = 2; // don't bother distinguishing 1-solution case, as code below will still work
                           }
                        }
                     } else {
                        float b = 3*(ax*bx + ay*by) * a_inv; // could precompute this as it doesn't depend on sample point
                        float c = (2*(ax*ax + ay*ay) + (mx*bx+my*by)) * a_inv;
                        float d = (mx*ax+my*ay) * a_inv;
                        num = nk_tt__solve_cubic(b, c, d, res);
                     }
                     if (num >= 1 && res[0] >= 0.0f && res[0] <= 1.0f) {
                        t = res[0], it = 1.0f - t;
                        px = it*it*x0 + 2*t*it*x1 + t*t*x2;
                        py = it*it*y0 + 2*t*it*y1 + t*t*y2;
                        dist2 = (px-sx)*(px-sx) + (py-sy)*(py-sy);
                        if (dist2 < min_dist * min_dist)
                           min_dist = (float) NK_TT_sqrt(dist2);
                     }
                     if (num >= 2 && res[1] >= 0.0f && res[1] <= 1.0f) {
                        t = res[1], it = 1.0f - t;
                        px = it*it*x0 + 2*t*it*x1 + t*t*x2;
                        py = it*it*y0 + 2*t*it*y1 + t*t*y2;
                        dist2 = (px-sx)*(px-sx) + (py-sy)*(py-sy);
                        if (dist2 < min_dist * min_dist)
                           min_dist = (float) NK_TT_sqrt(dist2);
                     }
                     if (num >= 3 && res[2] >= 0.0f && res[2] <= 1.0f) {
                        t = res[2], it = 1.0f - t;
                        px = it*it*x0 + 2*t*it*x1 + t*t*x2;
                        py = it*it*y0 + 2*t*it*y1 + t*t*y2;
                        dist2 = (px-sx)*(px-sx) + (py-sy)*(py-sy);
                        if (dist2 < min_dist * min_dist)
                           min_dist = (float) NK_TT_sqrt(dist2);
                     }
                  }
               }
            }
            if (winding == 0)
               min_dist = -min_dist;  // if outside the shape, value is negative
            val = onedge_value + pixel_dist_scale * min_dist;
            if (val < 0)
               val = 0;
            else if (val > 255)
               val = 255;
            data[(y-iy0)*w+(x-ix0)] = (unsigned char) val;
         }
      }
      NK_TT_free(precompute, info->userdata);
      NK_TT_free(verts, info->userdata);
   }
   return data;
}   

NK_TT_DEF unsigned char * nk_tt_GetCodepointSDF(const nk_tt_fontinfo *info, float scale, int codepoint, int padding, unsigned char onedge_value, float pixel_dist_scale, int *width, int *height, int *xoff, int *yoff)
{
   return nk_tt_GetGlyphSDF(info, scale, nk_tt_FindGlyphIndex(info, codepoint), padding, onedge_value, pixel_dist_scale, width, height, xoff, yoff);
}

NK_TT_DEF void nk_tt_FreeSDF(unsigned char *bitmap, void *userdata)
{
   NK_TT_free(bitmap, userdata);
}

//////////////////////////////////////////////////////////////////////////////
//
// font name matching -- recommended not to use this
//

// check if a utf8 string contains a prefix which is the utf16 string; if so return length of matching utf8 string
static nk_tt_int32 nk_tt__CompareUTF8toUTF16_bigendian_prefix(nk_tt_uint8 *s1, nk_tt_int32 len1, nk_tt_uint8 *s2, nk_tt_int32 len2) 
{
   nk_tt_int32 i=0;

   // convert utf16 to utf8 and compare the results while converting
   while (len2) {
      nk_tt_uint16 ch = s2[0]*256 + s2[1];
      if (ch < 0x80) {
         if (i >= len1) return -1;
         if (s1[i++] != ch) return -1;
      } else if (ch < 0x800) {
         if (i+1 >= len1) return -1;
         if (s1[i++] != 0xc0 + (ch >> 6)) return -1;
         if (s1[i++] != 0x80 + (ch & 0x3f)) return -1;
      } else if (ch >= 0xd800 && ch < 0xdc00) {
         nk_tt_uint32 c;
         nk_tt_uint16 ch2 = s2[2]*256 + s2[3];
         if (i+3 >= len1) return -1;
         c = ((ch - 0xd800) << 10) + (ch2 - 0xdc00) + 0x10000;
         if (s1[i++] != 0xf0 + (c >> 18)) return -1;
         if (s1[i++] != 0x80 + ((c >> 12) & 0x3f)) return -1;
         if (s1[i++] != 0x80 + ((c >>  6) & 0x3f)) return -1;
         if (s1[i++] != 0x80 + ((c      ) & 0x3f)) return -1;
         s2 += 2; // plus another 2 below
         len2 -= 2;
      } else if (ch >= 0xdc00 && ch < 0xe000) {
         return -1;
      } else {
         if (i+2 >= len1) return -1;
         if (s1[i++] != 0xe0 + (ch >> 12)) return -1;
         if (s1[i++] != 0x80 + ((ch >> 6) & 0x3f)) return -1;
         if (s1[i++] != 0x80 + ((ch     ) & 0x3f)) return -1;
      }
      s2 += 2;
      len2 -= 2;
   }
   return i;
}

static int nk_tt_CompareUTF8toUTF16_bigendian_internal(char *s1, int len1, char *s2, int len2) 
{
   return len1 == nk_tt__CompareUTF8toUTF16_bigendian_prefix((nk_tt_uint8*) s1, len1, (nk_tt_uint8*) s2, len2);
}

// returns results in whatever encoding you request... but note that 2-byte encodings
// will be BIG-ENDIAN... use nk_tt_CompareUTF8toUTF16_bigendian() to compare
NK_TT_DEF const char *nk_tt_GetFontNameString(const nk_tt_fontinfo *font, int *length, int platformID, int encodingID, int languageID, int nameID)
{
   nk_tt_int32 i,count,stringOffset;
   nk_tt_uint8 *fc = font->data;
   nk_tt_uint32 offset = font->fontstart;
   nk_tt_uint32 nm = nk_tt__find_table(fc, offset, "name");
   if (!nm) return NULL;

   count = ttUSHORT(fc+nm+2);
   stringOffset = nm + ttUSHORT(fc+nm+4);
   for (i=0; i < count; ++i) {
      nk_tt_uint32 loc = nm + 6 + 12 * i;
      if (platformID == ttUSHORT(fc+loc+0) && encodingID == ttUSHORT(fc+loc+2)
          && languageID == ttUSHORT(fc+loc+4) && nameID == ttUSHORT(fc+loc+6)) {
         *length = ttUSHORT(fc+loc+8);
         return (const char *) (fc+stringOffset+ttUSHORT(fc+loc+10));
      }
   }
   return NULL;
}

static int nk_tt__matchpair(nk_tt_uint8 *fc, nk_tt_uint32 nm, nk_tt_uint8 *name, nk_tt_int32 nlen, nk_tt_int32 target_id, nk_tt_int32 next_id)
{
   nk_tt_int32 i;
   nk_tt_int32 count = ttUSHORT(fc+nm+2);
   nk_tt_int32 stringOffset = nm + ttUSHORT(fc+nm+4);

   for (i=0; i < count; ++i) {
      nk_tt_uint32 loc = nm + 6 + 12 * i;
      nk_tt_int32 id = ttUSHORT(fc+loc+6);
      if (id == target_id) {
         // find the encoding
         nk_tt_int32 platform = ttUSHORT(fc+loc+0), encoding = ttUSHORT(fc+loc+2), language = ttUSHORT(fc+loc+4);

         // is this a Unicode encoding?
         if (platform == 0 || (platform == 3 && encoding == 1) || (platform == 3 && encoding == 10)) {
            nk_tt_int32 slen = ttUSHORT(fc+loc+8);
            nk_tt_int32 off = ttUSHORT(fc+loc+10);

            // check if there's a prefix match
            nk_tt_int32 matchlen = nk_tt__CompareUTF8toUTF16_bigendian_prefix(name, nlen, fc+stringOffset+off,slen);
            if (matchlen >= 0) {
               // check for target_id+1 immediately following, with same encoding & language
               if (i+1 < count && ttUSHORT(fc+loc+12+6) == next_id && ttUSHORT(fc+loc+12) == platform && ttUSHORT(fc+loc+12+2) == encoding && ttUSHORT(fc+loc+12+4) == language) {
                  slen = ttUSHORT(fc+loc+12+8);
                  off = ttUSHORT(fc+loc+12+10);
                  if (slen == 0) {
                     if (matchlen == nlen)
                        return 1;
                  } else if (matchlen < nlen && name[matchlen] == ' ') {
                     ++matchlen;
                     if (nk_tt_CompareUTF8toUTF16_bigendian_internal((char*) (name+matchlen), nlen-matchlen, (char*)(fc+stringOffset+off),slen))
                        return 1;
                  }
               } else {
                  // if nothing immediately following
                  if (matchlen == nlen)
                     return 1;
               }
            }
         }

         // @TODO handle other encodings
      }
   }
   return 0;
}

static int nk_tt__matches(nk_tt_uint8 *fc, nk_tt_uint32 offset, nk_tt_uint8 *name, nk_tt_int32 flags)
{
   nk_tt_int32 nlen = (nk_tt_int32) NK_TT_strlen((char *) name);
   nk_tt_uint32 nm,hd;
   if (!nk_tt__isfont(fc+offset)) return 0;

   // check italics/bold/underline flags in macStyle...
   if (flags) {
      hd = nk_tt__find_table(fc, offset, "head");
      if ((ttUSHORT(fc+hd+44) & 7) != (flags & 7)) return 0;
   }

   nm = nk_tt__find_table(fc, offset, "name");
   if (!nm) return 0;

   if (flags) {
      // if we checked the macStyle flags, then just check the family and ignore the subfamily
      if (nk_tt__matchpair(fc, nm, name, nlen, 16, -1))  return 1;
      if (nk_tt__matchpair(fc, nm, name, nlen,  1, -1))  return 1;
      if (nk_tt__matchpair(fc, nm, name, nlen,  3, -1))  return 1;
   } else {
      if (nk_tt__matchpair(fc, nm, name, nlen, 16, 17))  return 1;
      if (nk_tt__matchpair(fc, nm, name, nlen,  1,  2))  return 1;
      if (nk_tt__matchpair(fc, nm, name, nlen,  3, -1))  return 1;
   }

   return 0;
}

static int nk_tt_FindMatchingFont_internal(unsigned char *font_collection, char *name_utf8, nk_tt_int32 flags)
{
   nk_tt_int32 i;
   for (i=0;;++i) {
      nk_tt_int32 off = nk_tt_GetFontOffsetForIndex(font_collection, i);
      if (off < 0) return off;
      if (nk_tt__matches((nk_tt_uint8 *) font_collection, off, (nk_tt_uint8*) name_utf8, flags))
         return off;
   }
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

NK_TT_DEF int nk_tt_BakeFontBitmap(const unsigned char *data, int offset,
                                float pixel_height, unsigned char *pixels, int pw, int ph,
                                int first_char, int num_chars, nk_tt_bakedchar *chardata)
{
   return nk_tt_BakeFontBitmap_internal((unsigned char *) data, offset, pixel_height, pixels, pw, ph, first_char, num_chars, chardata);
}

NK_TT_DEF int nk_tt_GetFontOffsetForIndex(const unsigned char *data, int index)
{
   return nk_tt_GetFontOffsetForIndex_internal((unsigned char *) data, index);   
}

NK_TT_DEF int nk_tt_GetNumberOfFonts(const unsigned char *data)
{
   return nk_tt_GetNumberOfFonts_internal((unsigned char *) data);
}

NK_TT_DEF int nk_tt_InitFont(nk_tt_fontinfo *info, const unsigned char *data, int offset)
{
   return nk_tt_InitFont_internal(info, (unsigned char *) data, offset);
}

NK_TT_DEF int nk_tt_FindMatchingFont(const unsigned char *fontdata, const char *name, int flags)
{
   return nk_tt_FindMatchingFont_internal((unsigned char *) fontdata, (char *) name, flags);
}

NK_TT_DEF int nk_tt_CompareUTF8toUTF16_bigendian(const char *s1, int len1, const char *s2, int len2)
{
   return nk_tt_CompareUTF8toUTF16_bigendian_internal((char *) s1, len1, (char *) s2, len2);
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif // STB_TRUETYPE_IMPLEMENTATION


// FULL VERSION HISTORY
//
//   1.19 (2018-02-11) OpenType GPOS kerning (horizontal only), NK_TT_fmod
//   1.18 (2018-01-29) add missing function
//   1.17 (2017-07-23) make more arguments const; doc fix
//   1.16 (2017-07-12) SDF support
//   1.15 (2017-03-03) make more arguments const
//   1.14 (2017-01-16) num-fonts-in-TTC function
//   1.13 (2017-01-02) support OpenType fonts, certain Apple fonts
//   1.12 (2016-10-25) suppress warnings about casting away const with -Wcast-qual
//   1.11 (2016-04-02) fix unused-variable warning
//   1.10 (2016-04-02) allow user-defined fabs() replacement
//                     fix memory leak if fontsize=0.0
//                     fix warning from duplicate typedef
//   1.09 (2016-01-16) warning fix; avoid crash on outofmem; use alloc userdata for PackFontRanges
//   1.08 (2015-09-13) document nk_tt_Rasterize(); fixes for vertical & horizontal edges
//   1.07 (2015-08-01) allow PackFontRanges to accept arrays of sparse codepoints;
//                     allow PackFontRanges to pack and render in separate phases;
//                     fix nk_tt_GetFontOFfsetForIndex (never worked for non-0 input?);
//                     fixed an assert() bug in the new rasterizer
//                     replace assert() with NK_TT_assert() in new rasterizer
//   1.06 (2015-07-14) performance improvements (~35% faster on x86 and x64 on test machine)
//                     also more precise AA rasterizer, except if shapes overlap
//                     remove need for NK_TT_sort
//   1.05 (2015-04-15) fix misplaced definitions for NK_TT_STATIC
//   1.04 (2015-04-15) typo in example
//   1.03 (2015-04-12) NK_TT_STATIC, fix memory leak in new packing, various fixes
//   1.02 (2014-12-10) fix various warnings & compile issues w/ stb_rect_pack, C++
//   1.01 (2014-12-08) fix subpixel position when oversampling to exactly match
//                        non-oversampled; NK_TT_POINT_SIZE for packed case only
//   1.00 (2014-12-06) add new PackBegin etc. API, w/ support for oversampling
//   0.99 (2014-09-18) fix multiple bugs with subpixel rendering (ryg)
//   0.9  (2014-08-07) support certain mac/iOS fonts without an MS platformID
//   0.8b (2014-07-07) fix a warning
//   0.8  (2014-05-25) fix a few more warnings
//   0.7  (2013-09-25) bugfix: subpixel glyph bug fixed in 0.5 had come back
//   0.6c (2012-07-24) improve documentation
//   0.6b (2012-07-20) fix a few more warnings
//   0.6  (2012-07-17) fix warnings; added nk_tt_ScaleForMappingEmToPixels,
//                        nk_tt_GetFontBoundingBox, nk_tt_IsGlyphEmpty
//   0.5  (2011-12-09) bugfixes:
//                        subpixel glyph renderer computed wrong bounding box
//                        first vertex of shape can be off-curve (FreeSans)
//   0.4b (2011-12-03) fixed an error in the font baking example
//   0.4  (2011-12-01) kerning, subpixel rendering (tor)
//                    bugfixes for:
//                        codepoint-to-glyph conversion using table fmt=12
//                        codepoint-to-glyph conversion using table fmt=4
//                        nk_tt_GetBakedQuad with non-square texture (Zer)
//                    updated Hello World! sample to use kerning and subpixel
//                    fixed some warnings
//   0.3  (2009-06-24) cmap fmt=12, compound shapes (MM)
//                    userdata, malloc-from-userdata, non-zero fill (stb)
//   0.2  (2009-03-11) Fix unsigned/signed char warnings
//   0.1  (2009-03-09) First public release
//

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this 
software, either in source code form or as a compiled binary, for any purpose, 
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this 
software dedicate any and all copyright interest in the software to the public 
domain. We make this dedication for the benefit of the public at large and to 
the detriment of our heirs and successors. We intend this dedication to be an 
overt act of relinquishment in perpetuity of all present and future rights to 
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/

/* -------------------------------------------------------------
 *
 *                          FONT BAKING
 *
 * --------------------------------------------------------------*/
struct nk_font_bake_data {
    struct nk_tt_fontinfo info;
    struct nk_rp_rect *rects;
    nk_tt_pack_range *ranges;
    nk_rune range_count;
};

struct nk_font_baker {
    struct nk_allocator alloc;
    struct nk_tt_pack_context spc;
    struct nk_font_bake_data *build;
    nk_tt_packedchar *packed_chars;
    struct nk_rp_rect *rects;
    nk_tt_pack_range *ranges;
};

NK_GLOBAL const nk_size nk_rect_align = NK_ALIGNOF(struct nk_rp_rect);
NK_GLOBAL const nk_size nk_range_align = NK_ALIGNOF(nk_tt_pack_range);
NK_GLOBAL const nk_size nk_char_align = NK_ALIGNOF(nk_tt_packedchar);
NK_GLOBAL const nk_size nk_build_align = NK_ALIGNOF(struct nk_font_bake_data);
NK_GLOBAL const nk_size nk_baker_align = NK_ALIGNOF(struct nk_font_baker);

NK_INTERN int
nk_range_count(const nk_rune *range)
{
    const nk_rune *iter = range;
    NK_ASSERT(range);
    if (!range) return 0;
    while (*(iter++) != 0);
    return (iter == range) ? 0 : (int)((iter - range)/2);
}
NK_INTERN int
nk_range_glyph_count(const nk_rune *range, int count)
{
    int i = 0;
    int total_glyphs = 0;
    for (i = 0; i < count; ++i) {
        int diff;
        nk_rune f = range[(i*2)+0];
        nk_rune t = range[(i*2)+1];
        NK_ASSERT(t >= f);
        diff = (int)((t - f) + 1);
        total_glyphs += diff;
    }
    return total_glyphs;
}
NK_API const nk_rune*
nk_font_default_glyph_ranges(void)
{
    NK_STORAGE const nk_rune ranges[] = {0x0020, 0x00FF, 0};
    return ranges;
}
NK_API const nk_rune*
nk_font_chinese_glyph_ranges(void)
{
    NK_STORAGE const nk_rune ranges[] = {
        0x0020, 0x00FF,
        0x3000, 0x30FF,
        0x31F0, 0x31FF,
        0xFF00, 0xFFEF,
        0x4e00, 0x9FAF,
        0
    };
    return ranges;
}
NK_API const nk_rune*
nk_font_cyrillic_glyph_ranges(void)
{
    NK_STORAGE const nk_rune ranges[] = {
        0x0020, 0x00FF,
        0x0400, 0x052F,
        0x2DE0, 0x2DFF,
        0xA640, 0xA69F,
        0
    };
    return ranges;
}
NK_API const nk_rune*
nk_font_korean_glyph_ranges(void)
{
    NK_STORAGE const nk_rune ranges[] = {
        0x0020, 0x00FF,
        0x3131, 0x3163,
        0xAC00, 0xD79D,
        0
    };
    return ranges;
}
NK_INTERN void
nk_font_baker_memory(nk_size *temp, int *glyph_count,
    struct nk_font_config *config_list, int count)
{
    int range_count = 0;
    int total_range_count = 0;
    struct nk_font_config *iter, *i;

    NK_ASSERT(config_list);
    NK_ASSERT(glyph_count);
    if (!config_list) {
        *temp = 0;
        *glyph_count = 0;
        return;
    }
    *glyph_count = 0;
    for (iter = config_list; iter; iter = iter->next) {
        i = iter;
        do {if (!i->range) iter->range = nk_font_default_glyph_ranges();
            range_count = nk_range_count(i->range);
            total_range_count += range_count;
            *glyph_count += nk_range_glyph_count(i->range, range_count);
        } while ((i = i->n) != iter);
    }
    *temp = (nk_size)*glyph_count * sizeof(struct nk_rp_rect);
    *temp += (nk_size)total_range_count * sizeof(nk_tt_pack_range);
    *temp += (nk_size)*glyph_count * sizeof(nk_tt_packedchar);
    *temp += (nk_size)count * sizeof(struct nk_font_bake_data);
    *temp += sizeof(struct nk_font_baker);
    *temp += nk_rect_align + nk_range_align + nk_char_align;
    *temp += nk_build_align + nk_baker_align;
}
NK_INTERN struct nk_font_baker*
nk_font_baker(void *memory, int glyph_count, int count, struct nk_allocator *alloc)
{
    struct nk_font_baker *baker;
    if (!memory) return 0;
    /* setup baker inside a memory block  */
    baker = (struct nk_font_baker*)NK_ALIGN_PTR(memory, nk_baker_align);
    baker->build = (struct nk_font_bake_data*)NK_ALIGN_PTR((baker + 1), nk_build_align);
    baker->packed_chars = (nk_tt_packedchar*)NK_ALIGN_PTR((baker->build + count), nk_char_align);
    baker->rects = (struct nk_rp_rect*)NK_ALIGN_PTR((baker->packed_chars + glyph_count), nk_rect_align);
    baker->ranges = (nk_tt_pack_range*)NK_ALIGN_PTR((baker->rects + glyph_count), nk_range_align);
    baker->alloc = *alloc;
    return baker;
}
NK_INTERN int
nk_font_bake_pack(struct nk_font_baker *baker,
    nk_size *image_memory, int *width, int *height, struct nk_recti *custom,
    const struct nk_font_config *config_list, int count,
    struct nk_allocator *alloc)
{
    NK_STORAGE const nk_size max_height = 1024 * 32;
    const struct nk_font_config *config_iter, *it;
    int total_glyph_count = 0;
    int total_range_count = 0;
    int range_count = 0;
    int i = 0;

    NK_ASSERT(image_memory);
    NK_ASSERT(width);
    NK_ASSERT(height);
    NK_ASSERT(config_list);
    NK_ASSERT(count);
    NK_ASSERT(alloc);

    if (!image_memory || !width || !height || !config_list || !count) return nk_false;
    for (config_iter = config_list; config_iter; config_iter = config_iter->next) {
        it = config_iter;
        do {range_count = nk_range_count(it->range);
            total_range_count += range_count;
            total_glyph_count += nk_range_glyph_count(it->range, range_count);
        } while ((it = it->n) != config_iter);
    }
    /* setup font baker from temporary memory */
    for (config_iter = config_list; config_iter; config_iter = config_iter->next) {
        it = config_iter;
        do {if (!nk_tt_InitFont(&baker->build[i++].info, (const unsigned char*)it->ttf_blob, 0))
            return nk_false;
        } while ((it = it->n) != config_iter);
    }
    *height = 0;
    *width = (total_glyph_count > 1000) ? 1024 : 512;
    nk_tt_PackBegin(&baker->spc, 0, (int)*width, (int)max_height, 0, 1, alloc);
    {
        int input_i = 0;
        int range_n = 0;
        int rect_n = 0;
        int char_n = 0;

        if (custom) {
            /* pack custom user data first so it will be in the upper left corner*/
            struct nk_rp_rect custom_space;
            nk_zero(&custom_space, sizeof(custom_space));
            custom_space.w = (nk_rp_coord)(custom->w);
            custom_space.h = (nk_rp_coord)(custom->h);

            nk_tt_PackSetOversampling(&baker->spc, 1, 1);
            nk_rp_pack_rects((struct nk_rp_context*)baker->spc.pack_info, &custom_space, 1);
            *height = NK_MAX(*height, (int)(custom_space.y + custom_space.h));

            custom->x = (short)custom_space.x;
            custom->y = (short)custom_space.y;
            custom->w = (short)custom_space.w;
            custom->h = (short)custom_space.h;
        }

        /* first font pass: pack all glyphs */
        for (input_i = 0, config_iter = config_list; input_i < count && config_iter;
            config_iter = config_iter->next) {
            it = config_iter;
            do {int n = 0;
                int glyph_count;
                const nk_rune *in_range;
                const struct nk_font_config *cfg = it;
                struct nk_font_bake_data *tmp = &baker->build[input_i++];

                /* count glyphs + ranges in current font */
                glyph_count = 0; range_count = 0;
                for (in_range = cfg->range; in_range[0] && in_range[1]; in_range += 2) {
                    glyph_count += (int)(in_range[1] - in_range[0]) + 1;
                    range_count++;
                }

                /* setup ranges  */
                tmp->ranges = baker->ranges + range_n;
                tmp->range_count = (nk_rune)range_count;
                range_n += range_count;
                for (i = 0; i < range_count; ++i) {
                    in_range = &cfg->range[i * 2];
                    tmp->ranges[i].font_size = cfg->size;
                    tmp->ranges[i].first_unicode_codepoint_in_range = (int)in_range[0];
                    tmp->ranges[i].num_chars = (int)(in_range[1]- in_range[0]) + 1;
                    tmp->ranges[i].chardata_for_range = baker->packed_chars + char_n;
                    char_n += tmp->ranges[i].num_chars;
                }

                /* pack */
                tmp->rects = baker->rects + rect_n;
                rect_n += glyph_count;
                nk_tt_PackSetOversampling(&baker->spc, cfg->oversample_h, cfg->oversample_v);
                n = nk_tt_PackFontRangesGatherRects(&baker->spc, &tmp->info,
                    tmp->ranges, (int)tmp->range_count, tmp->rects);
                nk_rp_pack_rects((struct nk_rp_context*)baker->spc.pack_info, tmp->rects, (int)n);

                /* texture height */
                for (i = 0; i < n; ++i) {
                    if (tmp->rects[i].was_packed)
                        *height = NK_MAX(*height, tmp->rects[i].y + tmp->rects[i].h);
                }
            } while ((it = it->n) != config_iter);
        }
        NK_ASSERT(rect_n == total_glyph_count);
        NK_ASSERT(char_n == total_glyph_count);
        NK_ASSERT(range_n == total_range_count);
    }
    *height = (int)nk_round_up_pow2((nk_uint)*height);
    *image_memory = (nk_size)(*width) * (nk_size)(*height);
    return nk_true;
}
NK_INTERN void
nk_font_bake(struct nk_font_baker *baker, void *image_memory, int width, int height,
    struct nk_font_glyph *glyphs, int glyphs_count,
    const struct nk_font_config *config_list, int font_count)
{
    int input_i = 0;
    nk_rune glyph_n = 0;
    const struct nk_font_config *config_iter;
    const struct nk_font_config *it;

    NK_ASSERT(image_memory);
    NK_ASSERT(width);
    NK_ASSERT(height);
    NK_ASSERT(config_list);
    NK_ASSERT(baker);
    NK_ASSERT(font_count);
    NK_ASSERT(glyphs_count);
    if (!image_memory || !width || !height || !config_list ||
        !font_count || !glyphs || !glyphs_count)
        return;

    /* second font pass: render glyphs */
    nk_zero(image_memory, (nk_size)((nk_size)width * (nk_size)height));
    baker->spc.pixels = (unsigned char*)image_memory;
    baker->spc.height = (int)height;
    for (input_i = 0, config_iter = config_list; input_i < font_count && config_iter;
        config_iter = config_iter->next) {
        it = config_iter;
        do {const struct nk_font_config *cfg = it;
            struct nk_font_bake_data *tmp = &baker->build[input_i++];
            nk_tt_PackSetOversampling(&baker->spc, cfg->oversample_h, cfg->oversample_v);
            nk_tt_PackFontRangesRenderIntoRects(&baker->spc, &tmp->info, tmp->ranges,
                (int)tmp->range_count, tmp->rects);
        } while ((it = it->n) != config_iter);
    } nk_tt_PackEnd(&baker->spc);

    /* third pass: setup font and glyphs */
    for (input_i = 0, config_iter = config_list; input_i < font_count && config_iter;
        config_iter = config_iter->next) {
        it = config_iter;
        do {nk_size i = 0;
            int char_idx = 0;
            nk_rune glyph_count = 0;
            const struct nk_font_config *cfg = it;
            struct nk_font_bake_data *tmp = &baker->build[input_i++];
            struct nk_baked_font *dst_font = cfg->font;

            float font_scale = nk_tt_ScaleForPixelHeight(&tmp->info, cfg->size);
            int unscaled_ascent, unscaled_descent, unscaled_line_gap;
            nk_tt_GetFontVMetrics(&tmp->info, &unscaled_ascent, &unscaled_descent,
                                    &unscaled_line_gap);

            /* fill baked font */
            if (!cfg->merge_mode) {
                dst_font->ranges = cfg->range;
                dst_font->height = cfg->size;
                dst_font->ascent = ((float)unscaled_ascent * font_scale);
                dst_font->descent = ((float)unscaled_descent * font_scale);
                dst_font->glyph_offset = glyph_n;
                // Need to zero this, or it will carry over from a previous
                // bake, and cause a segfault when accessing glyphs[].
                dst_font->glyph_count = 0;
            }

            /* fill own baked font glyph array */
            for (i = 0; i < tmp->range_count; ++i) {
                nk_tt_pack_range *range = &tmp->ranges[i];
                for (char_idx = 0; char_idx < range->num_chars; char_idx++)
                {
                    nk_rune codepoint = 0;
                    float dummy_x = 0, dummy_y = 0;
                    nk_tt_aligned_quad q;
                    struct nk_font_glyph *glyph;

                    /* query glyph bounds from stb_truetype */
                    const nk_tt_packedchar *pc = &range->chardata_for_range[char_idx];
                    if (!pc->x0 && !pc->x1 && !pc->y0 && !pc->y1) continue;
                    codepoint = (nk_rune)(range->first_unicode_codepoint_in_range + char_idx);
                    nk_tt_GetPackedQuad(range->chardata_for_range, (int)width,
                        (int)height, char_idx, &dummy_x, &dummy_y, &q, 0);

                    /* fill own glyph type with data */
                    glyph = &glyphs[dst_font->glyph_offset + dst_font->glyph_count + (unsigned int)glyph_count];
                    glyph->codepoint = codepoint;
                    glyph->x0 = q.x0; glyph->y0 = q.y0;
                    glyph->x1 = q.x1; glyph->y1 = q.y1;
                    glyph->y0 += (dst_font->ascent + 0.5f);
                    glyph->y1 += (dst_font->ascent + 0.5f);
                    glyph->w = glyph->x1 - glyph->x0 + 0.5f;
                    glyph->h = glyph->y1 - glyph->y0;

                    if (cfg->coord_type == NK_COORD_PIXEL) {
                        glyph->u0 = q.s0 * (float)width;
                        glyph->v0 = q.t0 * (float)height;
                        glyph->u1 = q.s1 * (float)width;
                        glyph->v1 = q.t1 * (float)height;
                    } else {
                        glyph->u0 = q.s0;
                        glyph->v0 = q.t0;
                        glyph->u1 = q.s1;
                        glyph->v1 = q.t1;
                    }
                    glyph->xadvance = (pc->xadvance + cfg->spacing.x);
                    if (cfg->pixel_snap)
                        glyph->xadvance = (float)(int)(glyph->xadvance + 0.5f);
                    glyph_count++;
                }
            }
            dst_font->glyph_count += glyph_count;
            glyph_n += glyph_count;
        } while ((it = it->n) != config_iter);
    }
}
NK_INTERN void
nk_font_bake_custom_data(void *img_memory, int img_width, int img_height,
    struct nk_recti img_dst, const char *texture_data_mask, int tex_width,
    int tex_height, char white, char black)
{
    nk_byte *pixels;
    int y = 0;
    int x = 0;
    int n = 0;

    NK_ASSERT(img_memory);
    NK_ASSERT(img_width);
    NK_ASSERT(img_height);
    NK_ASSERT(texture_data_mask);
    NK_UNUSED(tex_height);
    if (!img_memory || !img_width || !img_height || !texture_data_mask)
        return;

    pixels = (nk_byte*)img_memory;
    for (y = 0, n = 0; y < tex_height; ++y) {
        for (x = 0; x < tex_width; ++x, ++n) {
            const int off0 = ((img_dst.x + x) + (img_dst.y + y) * img_width);
            const int off1 = off0 + 1 + tex_width;
            pixels[off0] = (texture_data_mask[n] == white) ? 0xFF : 0x00;
            pixels[off1] = (texture_data_mask[n] == black) ? 0xFF : 0x00;
        }
    }
}
NK_INTERN void
nk_font_bake_convert(void *out_memory, int img_width, int img_height,
    const void *in_memory)
{
    int n = 0;
    nk_rune *dst;
    const nk_byte *src;

    NK_ASSERT(out_memory);
    NK_ASSERT(in_memory);
    NK_ASSERT(img_width);
    NK_ASSERT(img_height);
    if (!out_memory || !in_memory || !img_height || !img_width) return;

    dst = (nk_rune*)out_memory;
    src = (const nk_byte*)in_memory;
    for (n = (int)(img_width * img_height); n > 0; n--)
        *dst++ = ((nk_rune)(*src++) << 24) | 0x00FFFFFF;
}

/* -------------------------------------------------------------
 *
 *                          FONT
 *
 * --------------------------------------------------------------*/
NK_INTERN float
nk_font_text_width(nk_handle handle, float height, const char *text, int len)
{
    nk_rune unicode;
    int text_len  = 0;
    float text_width = 0;
    int glyph_len = 0;
    float scale = 0;

    struct nk_font *font = (struct nk_font*)handle.ptr;
    NK_ASSERT(font);
    NK_ASSERT(font->glyphs);
    if (!font || !text || !len)
        return 0;

    scale = height/font->info.height;
    glyph_len = text_len = nk_utf_decode(text, &unicode, (int)len);
    if (!glyph_len) return 0;
    while (text_len <= (int)len && glyph_len) {
        const struct nk_font_glyph *g;
        if (unicode == NK_UTF_INVALID) break;

        /* query currently drawn glyph information */
        g = nk_font_find_glyph(font, unicode);
        text_width += g->xadvance * scale;

        /* offset next glyph */
        glyph_len = nk_utf_decode(text + text_len, &unicode, (int)len - text_len);
        text_len += glyph_len;
    }
    return text_width;
}
#ifdef NK_INCLUDE_VERTEX_BUFFER_OUTPUT
NK_INTERN void
nk_font_query_font_glyph(nk_handle handle, float height,
    struct nk_user_font_glyph *glyph, nk_rune codepoint, nk_rune next_codepoint)
{
    float scale;
    const struct nk_font_glyph *g;
    struct nk_font *font;

    NK_ASSERT(glyph);
    NK_UNUSED(next_codepoint);

    font = (struct nk_font*)handle.ptr;
    NK_ASSERT(font);
    NK_ASSERT(font->glyphs);
    if (!font || !glyph)
        return;

    scale = height/font->info.height;
    g = nk_font_find_glyph(font, codepoint);
    glyph->width = (g->x1 - g->x0) * scale;
    glyph->height = (g->y1 - g->y0) * scale;
    glyph->offset = nk_vec2(g->x0 * scale, g->y0 * scale);
    glyph->xadvance = (g->xadvance * scale);
    glyph->uv[0] = nk_vec2(g->u0, g->v0);
    glyph->uv[1] = nk_vec2(g->u1, g->v1);
}
#endif
NK_API const struct nk_font_glyph*
nk_font_find_glyph(struct nk_font *font, nk_rune unicode)
{
    int i = 0;
    int count;
    int total_glyphs = 0;
    const struct nk_font_glyph *glyph = 0;
    const struct nk_font_config *iter = 0;

    NK_ASSERT(font);
    NK_ASSERT(font->glyphs);
    NK_ASSERT(font->info.ranges);
    if (!font || !font->glyphs) return 0;

    glyph = font->fallback;
    iter = font->config;
    do {count = nk_range_count(iter->range);
        for (i = 0; i < count; ++i) {
            nk_rune f = iter->range[(i*2)+0];
            nk_rune t = iter->range[(i*2)+1];
            int diff = (int)((t - f) + 1);
            if (unicode >= f && unicode <= t)
                return &font->glyphs[((nk_rune)total_glyphs + (unicode - f))];
            total_glyphs += diff;
        }
    } while ((iter = iter->n) != font->config);
    return glyph;
}
NK_INTERN void
nk_font_init(struct nk_font *font, float pixel_height,
    nk_rune fallback_codepoint, struct nk_font_glyph *glyphs,
    const struct nk_baked_font *baked_font, nk_handle atlas)
{
    struct nk_baked_font baked;
    NK_ASSERT(font);
    NK_ASSERT(glyphs);
    NK_ASSERT(baked_font);
    if (!font || !glyphs || !baked_font)
        return;

    baked = *baked_font;
    font->fallback = 0;
    font->info = baked;
    font->scale = (float)pixel_height / (float)font->info.height;
    font->glyphs = &glyphs[baked_font->glyph_offset];
    font->texture = atlas;
    font->fallback_codepoint = fallback_codepoint;
    font->fallback = nk_font_find_glyph(font, fallback_codepoint);

    font->handle.height = font->info.height * font->scale;
    font->handle.width = nk_font_text_width;
    font->handle.userdata.ptr = font;
#ifdef NK_INCLUDE_VERTEX_BUFFER_OUTPUT
    font->handle.query = nk_font_query_font_glyph;
    font->handle.texture = font->texture;
#endif
}

/* ---------------------------------------------------------------------------
 *
 *                          DEFAULT FONT
 *
 * ProggyClean.ttf
 * Copyright (c) 2004, 2005 Tristan Grimmer
 * MIT license (see License.txt in http://www.upperbounds.net/download/ProggyClean.ttf.zip)
 * Download and more information at http://upperbounds.net
 *-----------------------------------------------------------------------------*/
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverlength-strings"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#endif

#ifdef NK_INCLUDE_DEFAULT_FONT

NK_GLOBAL const char nk_proggy_clean_ttf_compressed_data_base85[11980+1] =
    "7])#######hV0qs'/###[),##/l:$#Q6>##5[n42>c-TH`->>#/e>11NNV=Bv(*:.F?uu#(gRU.o0XGH`$vhLG1hxt9?W`#,5LsCp#-i>.r$<$6pD>Lb';9Crc6tgXmKVeU2cD4Eo3R/"
    "2*>]b(MC;$jPfY.;h^`IWM9<Lh2TlS+f-s$o6Q<BWH`YiU.xfLq$N;$0iR/GX:U(jcW2p/W*q?-qmnUCI;jHSAiFWM.R*kU@C=GH?a9wp8f$e.-4^Qg1)Q-GL(lf(r/7GrRgwV%MS=C#"
    "`8ND>Qo#t'X#(v#Y9w0#1D$CIf;W'#pWUPXOuxXuU(H9M(1<q-UE31#^-V'8IRUo7Qf./L>=Ke$$'5F%)]0^#0X@U.a<r:QLtFsLcL6##lOj)#.Y5<-R&KgLwqJfLgN&;Q?gI^#DY2uL"
    "i@^rMl9t=cWq6##weg>$FBjVQTSDgEKnIS7EM9>ZY9w0#L;>>#Mx&4Mvt//L[MkA#W@lK.N'[0#7RL_&#w+F%HtG9M#XL`N&.,GM4Pg;-<nLENhvx>-VsM.M0rJfLH2eTM`*oJMHRC`N"
    "kfimM2J,W-jXS:)r0wK#@Fge$U>`w'N7G#$#fB#$E^$#:9:hk+eOe--6x)F7*E%?76%^GMHePW-Z5l'&GiF#$956:rS?dA#fiK:)Yr+`&#0j@'DbG&#^$PG.Ll+DNa<XCMKEV*N)LN/N"
    "*b=%Q6pia-Xg8I$<MR&,VdJe$<(7G;Ckl'&hF;;$<_=X(b.RS%%)###MPBuuE1V:v&cX&#2m#(&cV]`k9OhLMbn%s$G2,B$BfD3X*sp5#l,$R#]x_X1xKX%b5U*[r5iMfUo9U`N99hG)"
    "tm+/Us9pG)XPu`<0s-)WTt(gCRxIg(%6sfh=ktMKn3j)<6<b5Sk_/0(^]AaN#(p/L>&VZ>1i%h1S9u5o@YaaW$e+b<TWFn/Z:Oh(Cx2$lNEoN^e)#CFY@@I;BOQ*sRwZtZxRcU7uW6CX"
    "ow0i(?$Q[cjOd[P4d)]>ROPOpxTO7Stwi1::iB1q)C_=dV26J;2,]7op$]uQr@_V7$q^%lQwtuHY]=DX,n3L#0PHDO4f9>dC@O>HBuKPpP*E,N+b3L#lpR/MrTEH.IAQk.a>D[.e;mc."
    "x]Ip.PH^'/aqUO/$1WxLoW0[iLA<QT;5HKD+@qQ'NQ(3_PLhE48R.qAPSwQ0/WK?Z,[x?-J;jQTWA0X@KJ(_Y8N-:/M74:/-ZpKrUss?d#dZq]DAbkU*JqkL+nwX@@47`5>w=4h(9.`G"
    "CRUxHPeR`5Mjol(dUWxZa(>STrPkrJiWx`5U7F#.g*jrohGg`cg:lSTvEY/EV_7H4Q9[Z%cnv;JQYZ5q.l7Zeas:HOIZOB?G<Nald$qs]@]L<J7bR*>gv:[7MI2k).'2($5FNP&EQ(,)"
    "U]W]+fh18.vsai00);D3@4ku5P?DP8aJt+;qUM]=+b'8@;mViBKx0DE[-auGl8:PJ&Dj+M6OC]O^((##]`0i)drT;-7X`=-H3[igUnPG-NZlo.#k@h#=Ork$m>a>$-?Tm$UV(?#P6YY#"
    "'/###xe7q.73rI3*pP/$1>s9)W,JrM7SN]'/4C#v$U`0#V.[0>xQsH$fEmPMgY2u7Kh(G%siIfLSoS+MK2eTM$=5,M8p`A.;_R%#u[K#$x4AG8.kK/HSB==-'Ie/QTtG?-.*^N-4B/ZM"
    "_3YlQC7(p7q)&](`6_c)$/*JL(L-^(]$wIM`dPtOdGA,U3:w2M-0<q-]L_?^)1vw'.,MRsqVr.L;aN&#/EgJ)PBc[-f>+WomX2u7lqM2iEumMTcsF?-aT=Z-97UEnXglEn1K-bnEO`gu"
    "Ft(c%=;Am_Qs@jLooI&NX;]0#j4#F14;gl8-GQpgwhrq8'=l_f-b49'UOqkLu7-##oDY2L(te+Mch&gLYtJ,MEtJfLh'x'M=$CS-ZZ%P]8bZ>#S?YY#%Q&q'3^Fw&?D)UDNrocM3A76/"
    "/oL?#h7gl85[qW/NDOk%16ij;+:1a'iNIdb-ou8.P*w,v5#EI$TWS>Pot-R*H'-SEpA:g)f+O$%%`kA#G=8RMmG1&O`>to8bC]T&$,n.LoO>29sp3dt-52U%VM#q7'DHpg+#Z9%H[K<L"
    "%a2E-grWVM3@2=-k22tL]4$##6We'8UJCKE[d_=%wI;'6X-GsLX4j^SgJ$##R*w,vP3wK#iiW&#*h^D&R?jp7+/u&#(AP##XU8c$fSYW-J95_-Dp[g9wcO&#M-h1OcJlc-*vpw0xUX&#"
    "OQFKNX@QI'IoPp7nb,QU//MQ&ZDkKP)X<WSVL(68uVl&#c'[0#(s1X&xm$Y%B7*K:eDA323j998GXbA#pwMs-jgD$9QISB-A_(aN4xoFM^@C58D0+Q+q3n0#3U1InDjF682-SjMXJK)("
    "h$hxua_K]ul92%'BOU&#BRRh-slg8KDlr:%L71Ka:.A;%YULjDPmL<LYs8i#XwJOYaKPKc1h:'9Ke,g)b),78=I39B;xiY$bgGw-&.Zi9InXDuYa%G*f2Bq7mn9^#p1vv%#(Wi-;/Z5h"
    "o;#2:;%d&#x9v68C5g?ntX0X)pT`;%pB3q7mgGN)3%(P8nTd5L7GeA-GL@+%J3u2:(Yf>et`e;)f#Km8&+DC$I46>#Kr]]u-[=99tts1.qb#q72g1WJO81q+eN'03'eM>&1XxY-caEnO"
    "j%2n8)),?ILR5^.Ibn<-X-Mq7[a82Lq:F&#ce+S9wsCK*x`569E8ew'He]h:sI[2LM$[guka3ZRd6:t%IG:;$%YiJ:Nq=?eAw;/:nnDq0(CYcMpG)qLN4$##&J<j$UpK<Q4a1]MupW^-"
    "sj_$%[HK%'F####QRZJ::Y3EGl4'@%FkiAOg#p[##O`gukTfBHagL<LHw%q&OV0##F=6/:chIm0@eCP8X]:kFI%hl8hgO@RcBhS-@Qb$%+m=hPDLg*%K8ln(wcf3/'DW-$.lR?n[nCH-"
    "eXOONTJlh:.RYF%3'p6sq:UIMA945&^HFS87@$EP2iG<-lCO$%c`uKGD3rC$x0BL8aFn--`ke%#HMP'vh1/R&O_J9'um,.<tx[@%wsJk&bUT2`0uMv7gg#qp/ij.L56'hl;.s5CUrxjO"
    "M7-##.l+Au'A&O:-T72L]P`&=;ctp'XScX*rU.>-XTt,%OVU4)S1+R-#dg0/Nn?Ku1^0f$B*P:Rowwm-`0PKjYDDM'3]d39VZHEl4,.j']Pk-M.h^&:0FACm$maq-&sgw0t7/6(^xtk%"
    "LuH88Fj-ekm>GA#_>568x6(OFRl-IZp`&b,_P'$M<Jnq79VsJW/mWS*PUiq76;]/NM_>hLbxfc$mj`,O;&%W2m`Zh:/)Uetw:aJ%]K9h:TcF]u_-Sj9,VK3M.*'&0D[Ca]J9gp8,kAW]"
    "%(?A%R$f<->Zts'^kn=-^@c4%-pY6qI%J%1IGxfLU9CP8cbPlXv);C=b),<2mOvP8up,UVf3839acAWAW-W?#ao/^#%KYo8fRULNd2.>%m]UK:n%r$'sw]J;5pAoO_#2mO3n,'=H5(et"
    "Hg*`+RLgv>=4U8guD$I%D:W>-r5V*%j*W:Kvej.Lp$<M-SGZ':+Q_k+uvOSLiEo(<aD/K<CCc`'Lx>'?;++O'>()jLR-^u68PHm8ZFWe+ej8h:9r6L*0//c&iH&R8pRbA#Kjm%upV1g:"
    "a_#Ur7FuA#(tRh#.Y5K+@?3<-8m0$PEn;J:rh6?I6uG<-`wMU'ircp0LaE_OtlMb&1#6T.#FDKu#1Lw%u%+GM+X'e?YLfjM[VO0MbuFp7;>Q&#WIo)0@F%q7c#4XAXN-U&VB<HFF*qL("
    "$/V,;(kXZejWO`<[5?\?ewY(*9=%wDc;,u<'9t3W-(H1th3+G]ucQ]kLs7df($/*JL]@*t7Bu_G3_7mp7<iaQjO@.kLg;x3B0lqp7Hf,^Ze7-##@/c58Mo(3;knp0%)A7?-W+eI'o8)b<"
    "nKnw'Ho8C=Y>pqB>0ie&jhZ[?iLR@@_AvA-iQC(=ksRZRVp7`.=+NpBC%rh&3]R:8XDmE5^V8O(x<<aG/1N$#FX$0V5Y6x'aErI3I$7x%E`v<-BY,)%-?Psf*l?%C3.mM(=/M0:JxG'?"
    "7WhH%o'a<-80g0NBxoO(GH<dM]n.+%q@jH?f.UsJ2Ggs&4<-e47&Kl+f//9@`b+?.TeN_&B8Ss?v;^Trk;f#YvJkl&w$]>-+k?'(<S:68tq*WoDfZu';mM?8X[ma8W%*`-=;D.(nc7/;"
    ")g:T1=^J$&BRV(-lTmNB6xqB[@0*o.erM*<SWF]u2=st-*(6v>^](H.aREZSi,#1:[IXaZFOm<-ui#qUq2$##Ri;u75OK#(RtaW-K-F`S+cF]uN`-KMQ%rP/Xri.LRcB##=YL3BgM/3M"
    "D?@f&1'BW-)Ju<L25gl8uhVm1hL$##*8###'A3/LkKW+(^rWX?5W_8g)a(m&K8P>#bmmWCMkk&#TR`C,5d>g)F;t,4:@_l8G/5h4vUd%&%950:VXD'QdWoY-F$BtUwmfe$YqL'8(PWX("
    "P?^@Po3$##`MSs?DWBZ/S>+4%>fX,VWv/w'KD`LP5IbH;rTV>n3cEK8U#bX]l-/V+^lj3;vlMb&[5YQ8#pekX9JP3XUC72L,,?+Ni&co7ApnO*5NK,((W-i:$,kp'UDAO(G0Sq7MVjJs"
    "bIu)'Z,*[>br5fX^:FPAWr-m2KgL<LUN098kTF&#lvo58=/vjDo;.;)Ka*hLR#/k=rKbxuV`>Q_nN6'8uTG&#1T5g)uLv:873UpTLgH+#FgpH'_o1780Ph8KmxQJ8#H72L4@768@Tm&Q"
    "h4CB/5OvmA&,Q&QbUoi$a_%3M01H)4x7I^&KQVgtFnV+;[Pc>[m4k//,]1?#`VY[Jr*3&&slRfLiVZJ:]?=K3Sw=[$=uRB?3xk48@aeg<Z'<$#4H)6,>e0jT6'N#(q%.O=?2S]u*(m<-"
    "V8J'(1)G][68hW$5'q[GC&5j`TE?m'esFGNRM)j,ffZ?-qx8;->g4t*:CIP/[Qap7/9'#(1sao7w-.qNUdkJ)tCF&#B^;xGvn2r9FEPFFFcL@.iFNkTve$m%#QvQS8U@)2Z+3K:AKM5i"
    "sZ88+dKQ)W6>J%CL<KE>`.d*(B`-n8D9oK<Up]c$X$(,)M8Zt7/[rdkqTgl-0cuGMv'?>-XV1q['-5k'cAZ69e;D_?$ZPP&s^+7])$*$#@QYi9,5P&#9r+$%CE=68>K8r0=dSC%%(@p7"
    ".m7jilQ02'0-VWAg<a/''3u.=4L$Y)6k/K:_[3=&jvL<L0C/2'v:^;-DIBW,B4E68:kZ;%?8(Q8BH=kO65BW?xSG&#@uU,DS*,?.+(o(#1vCS8#CHF>TlGW'b)Tq7VT9q^*^$$.:&N@@"
    "$&)WHtPm*5_rO0&e%K&#-30j(E4#'Zb.o/(Tpm$>K'f@[PvFl,hfINTNU6u'0pao7%XUp9]5.>%h`8_=VYbxuel.NTSsJfLacFu3B'lQSu/m6-Oqem8T+oE--$0a/k]uj9EwsG>%veR*"
    "hv^BFpQj:K'#SJ,sB-'#](j.Lg92rTw-*n%@/;39rrJF,l#qV%OrtBeC6/,;qB3ebNW[?,Hqj2L.1NP&GjUR=1D8QaS3Up&@*9wP?+lo7b?@%'k4`p0Z$22%K3+iCZj?XJN4Nm&+YF]u"
    "@-W$U%VEQ/,,>>#)D<h#`)h0:<Q6909ua+&VU%n2:cG3FJ-%@Bj-DgLr`Hw&HAKjKjseK</xKT*)B,N9X3]krc12t'pgTV(Lv-tL[xg_%=M_q7a^x?7Ubd>#%8cY#YZ?=,`Wdxu/ae&#"
    "w6)R89tI#6@s'(6Bf7a&?S=^ZI_kS&ai`&=tE72L_D,;^R)7[$s<Eh#c&)q.MXI%#v9ROa5FZO%sF7q7Nwb&#ptUJ:aqJe$Sl68%.D###EC><?-aF&#RNQv>o8lKN%5/$(vdfq7+ebA#"
    "u1p]ovUKW&Y%q]'>$1@-[xfn$7ZTp7mM,G,Ko7a&Gu%G[RMxJs[0MM%wci.LFDK)(<c`Q8N)jEIF*+?P2a8g%)$q]o2aH8C&<SibC/q,(e:v;-b#6[$NtDZ84Je2KNvB#$P5?tQ3nt(0"
    "d=j.LQf./Ll33+(;q3L-w=8dX$#WF&uIJ@-bfI>%:_i2B5CsR8&9Z&#=mPEnm0f`<&c)QL5uJ#%u%lJj+D-r;BoF&#4DoS97h5g)E#o:&S4weDF,9^Hoe`h*L+_a*NrLW-1pG_&2UdB8"
    "6e%B/:=>)N4xeW.*wft-;$'58-ESqr<b?UI(_%@[P46>#U`'6AQ]m&6/`Z>#S?YY#Vc;r7U2&326d=w&H####?TZ`*4?&.MK?LP8Vxg>$[QXc%QJv92.(Db*B)gb*BM9dM*hJMAo*c&#"
    "b0v=Pjer]$gG&JXDf->'StvU7505l9$AFvgYRI^&<^b68?j#q9QX4SM'RO#&sL1IM.rJfLUAj221]d##DW=m83u5;'bYx,*Sl0hL(W;;$doB&O/TQ:(Z^xBdLjL<Lni;''X.`$#8+1GD"
    ":k$YUWsbn8ogh6rxZ2Z9]%nd+>V#*8U_72Lh+2Q8Cj0i:6hp&$C/:p(HK>T8Y[gHQ4`4)'$Ab(Nof%V'8hL&#<NEdtg(n'=S1A(Q1/I&4([%dM`,Iu'1:_hL>SfD07&6D<fp8dHM7/g+"
    "tlPN9J*rKaPct&?'uBCem^jn%9_K)<,C5K3s=5g&GmJb*[SYq7K;TRLGCsM-$$;S%:Y@r7AK0pprpL<Lrh,q7e/%KWK:50I^+m'vi`3?%Zp+<-d+$L-Sv:@.o19n$s0&39;kn;S%BSq*"
    "$3WoJSCLweV[aZ'MQIjO<7;X-X;&+dMLvu#^UsGEC9WEc[X(wI7#2.(F0jV*eZf<-Qv3J-c+J5AlrB#$p(H68LvEA'q3n0#m,[`*8Ft)FcYgEud]CWfm68,(aLA$@EFTgLXoBq/UPlp7"
    ":d[/;r_ix=:TF`S5H-b<LI&HY(K=h#)]Lk$K14lVfm:x$H<3^Ql<M`$OhapBnkup'D#L$Pb_`N*g]2e;X/Dtg,bsj&K#2[-:iYr'_wgH)NUIR8a1n#S?Yej'h8^58UbZd+^FKD*T@;6A"
    "7aQC[K8d-(v6GI$x:T<&'Gp5Uf>@M.*J:;$-rv29'M]8qMv-tLp,'886iaC=Hb*YJoKJ,(j%K=H`K.v9HggqBIiZu'QvBT.#=)0ukruV&.)3=(^1`o*Pj4<-<aN((^7('#Z0wK#5GX@7"
    "u][`*S^43933A4rl][`*O4CgLEl]v$1Q3AeF37dbXk,.)vj#x'd`;qgbQR%FW,2(?LO=s%Sc68%NP'##Aotl8x=BE#j1UD([3$M(]UI2LX3RpKN@;/#f'f/&_mt&F)XdF<9t4)Qa.*kT"
    "LwQ'(TTB9.xH'>#MJ+gLq9-##@HuZPN0]u:h7.T..G:;$/Usj(T7`Q8tT72LnYl<-qx8;-HV7Q-&Xdx%1a,hC=0u+HlsV>nuIQL-5<N?)NBS)QN*_I,?&)2'IM%L3I)X((e/dl2&8'<M"
    ":^#M*Q+[T.Xri.LYS3v%fF`68h;b-X[/En'CR.q7E)p'/kle2HM,u;^%OKC-N+Ll%F9CF<Nf'^#t2L,;27W:0O@6##U6W7:$rJfLWHj$#)woqBefIZ.PK<b*t7ed;p*_m;4ExK#h@&]>"
    "_>@kXQtMacfD.m-VAb8;IReM3$wf0''hra*so568'Ip&vRs849'MRYSp%:t:h5qSgwpEr$B>Q,;s(C#$)`svQuF$##-D,##,g68@2[T;.XSdN9Qe)rpt._K-#5wF)sP'##p#C0c%-Gb%"
    "hd+<-j'Ai*x&&HMkT]C'OSl##5RG[JXaHN;d'uA#x._U;.`PU@(Z3dt4r152@:v,'R.Sj'w#0<-;kPI)FfJ&#AYJ&#//)>-k=m=*XnK$>=)72L]0I%>.G690a:$##<,);?;72#?x9+d;"
    "^V'9;jY@;)br#q^YQpx:X#Te$Z^'=-=bGhLf:D6&bNwZ9-ZD#n^9HhLMr5G;']d&6'wYmTFmL<LD)F^%[tC'8;+9E#C$g%#5Y>q9wI>P(9mI[>kC-ekLC/R&CH+s'B;K-M6$EB%is00:"
    "+A4[7xks.LrNk0&E)wILYF@2L'0Nb$+pv<(2.768/FrY&h$^3i&@+G%JT'<-,v`3;_)I9M^AE]CN?Cl2AZg+%4iTpT3<n-&%H%b<FDj2M<hH=&Eh<2Len$b*aTX=-8QxN)k11IM1c^j%"
    "9s<L<NFSo)B?+<-(GxsF,^-Eh@$4dXhN$+#rxK8'je'D7k`e;)2pYwPA'_p9&@^18ml1^[@g4t*[JOa*[=Qp7(qJ_oOL^('7fB&Hq-:sf,sNj8xq^>$U4O]GKx'm9)b@p7YsvK3w^YR-"
    "CdQ*:Ir<($u&)#(&?L9Rg3H)4fiEp^iI9O8KnTj,]H?D*r7'M;PwZ9K0E^k&-cpI;.p/6_vwoFMV<->#%Xi.LxVnrU(4&8/P+:hLSKj$#U%]49t'I:rgMi'FL@a:0Y-uA[39',(vbma*"
    "hU%<-SRF`Tt:542R_VV$p@[p8DV[A,?1839FWdF<TddF<9Ah-6&9tWoDlh]&1SpGMq>Ti1O*H&#(AL8[_P%.M>v^-))qOT*F5Cq0`Ye%+$B6i:7@0IX<N+T+0MlMBPQ*Vj>SsD<U4JHY"
    "8kD2)2fU/M#$e.)T4,_=8hLim[&);?UkK'-x?'(:siIfL<$pFM`i<?%W(mGDHM%>iWP,##P`%/L<eXi:@Z9C.7o=@(pXdAO/NLQ8lPl+HPOQa8wD8=^GlPa8TKI1CjhsCTSLJM'/Wl>-"
    "S(qw%sf/@%#B6;/U7K]uZbi^Oc^2n<bhPmUkMw>%t<)'mEVE''n`WnJra$^TKvX5B>;_aSEK',(hwa0:i4G?.Bci.(X[?b*($,=-n<.Q%`(X=?+@Am*Js0&=3bh8K]mL<LoNs'6,'85`"
    "0?t/'_U59@]ddF<#LdF<eWdF<OuN/45rY<-L@&#+fm>69=Lb,OcZV/);TTm8VI;?%OtJ<(b4mq7M6:u?KRdF<gR@2L=FNU-<b[(9c/ML3m;Z[$oF3g)GAWqpARc=<ROu7cL5l;-[A]%/"
    "+fsd;l#SafT/f*W]0=O'$(Tb<[)*@e775R-:Yob%g*>l*:xP?Yb.5)%w_I?7uk5JC+FS(m#i'k.'a0i)9<7b'fs'59hq$*5Uhv##pi^8+hIEBF`nvo`;'l0.^S1<-wUK2/Coh58KKhLj"
    "M=SO*rfO`+qC`W-On.=AJ56>>i2@2LH6A:&5q`?9I3@@'04&p2/LVa*T-4<-i3;M9UvZd+N7>b*eIwg:CC)c<>nO&#<IGe;__.thjZl<%w(Wk2xmp4Q@I#I9,DF]u7-P=.-_:YJ]aS@V"
    "?6*C()dOp7:WL,b&3Rg/.cmM9&r^>$(>.Z-I&J(Q0Hd5Q%7Co-b`-c<N(6r@ip+AurK<m86QIth*#v;-OBqi+L7wDE-Ir8K['m+DDSLwK&/.?-V%U_%3:qKNu$_b*B-kp7NaD'QdWQPK"
    "Yq[@>P)hI;*_F]u`Rb[.j8_Q/<&>uu+VsH$sM9TA%?)(vmJ80),P7E>)tjD%2L=-t#fK[%`v=Q8<FfNkgg^oIbah*#8/Qt$F&:K*-(N/'+1vMB,u()-a.VUU*#[e%gAAO(S>WlA2);Sa"
    ">gXm8YB`1d@K#n]76-a$U,mF<fX]idqd)<3,]J7JmW4`6]uks=4-72L(jEk+:bJ0M^q-8Dm_Z?0olP1C9Sa&H[d&c$ooQUj]Exd*3ZM@-WGW2%s',B-_M%>%Ul:#/'xoFM9QX-$.QN'>"
    "[%$Z$uF6pA6Ki2O5:8w*vP1<-1`[G,)-m#>0`P&#eb#.3i)rtB61(o'$?X3B</R90;eZ]%Ncq;-Tl]#F>2Qft^ae_5tKL9MUe9b*sLEQ95C&`=G?@Mj=wh*'3E>=-<)Gt*Iw)'QG:`@I"
    "wOf7&]1i'S01B+Ev/Nac#9S;=;YQpg_6U`*kVY39xK,[/6Aj7:'1Bm-_1EYfa1+o&o4hp7KN_Q(OlIo@S%;jVdn0'1<Vc52=u`3^o-n1'g4v58Hj&6_t7$##?M)c<$bgQ_'SY((-xkA#"
    "Y(,p'H9rIVY-b,'%bCPF7.J<Up^,(dU1VY*5#WkTU>h19w,WQhLI)3S#f$2(eb,jr*b;3Vw]*7NH%$c4Vs,eD9>XW8?N]o+(*pgC%/72LV-u<Hp,3@e^9UB1J+ak9-TN/mhKPg+AJYd$"
    "MlvAF_jCK*.O-^(63adMT->W%iewS8W6m2rtCpo'RS1R84=@paTKt)>=%&1[)*vp'u+x,VrwN;&]kuO9JDbg=pO$J*.jVe;u'm0dr9l,<*wMK*Oe=g8lV_KEBFkO'oU]^=[-792#ok,)"
    "i]lR8qQ2oA8wcRCZ^7w/Njh;?.stX?Q1>S1q4Bn$)K1<-rGdO'$Wr.Lc.CG)$/*JL4tNR/,SVO3,aUw'DJN:)Ss;wGn9A32ijw%FL+Z0Fn.U9;reSq)bmI32U==5ALuG&#Vf1398/pVo"
    "1*c-(aY168o<`JsSbk-,1N;$>0:OUas(3:8Z972LSfF8eb=c-;>SPw7.6hn3m`9^Xkn(r.qS[0;T%&Qc=+STRxX'q1BNk3&*eu2;&8q$&x>Q#Q7^Tf+6<(d%ZVmj2bDi%.3L2n+4W'$P"
    "iDDG)g,r%+?,$@?uou5tSe2aN_AQU*<h`e-GI7)?OK2A.d7_c)?wQ5AS@DL3r#7fSkgl6-++D:'A,uq7SvlB$pcpH'q3n0#_%dY#xCpr-l<F0NR@-##FEV6NTF6##$l84N1w?AO>'IAO"
    "URQ##V^Fv-XFbGM7Fl(N<3DhLGF%q.1rC$#:T__&Pi68%0xi_&[qFJ(77j_&JWoF.V735&T,[R*:xFR*K5>>#`bW-?4Ne_&6Ne_&6Ne_&n`kr-#GJcM6X;uM6X;uM(.a..^2TkL%oR(#"
    ";u.T%fAr%4tJ8&><1=GHZ_+m9/#H1F^R#SC#*N=BA9(D?v[UiFY>>^8p,KKF.W]L29uLkLlu/+4T<XoIB&hx=T1PcDaB&;HH+-AFr?(m9HZV)FKS8JCw;SD=6[^/DZUL`EUDf]GGlG&>"
    "w$)F./^n3+rlo+DB;5sIYGNk+i1t-69Jg--0pao7Sm#K)pdHW&;LuDNH@H>#/X-TI(;P>#,Gc>#0Su>#4`1?#8lC?#<xU?#@.i?#D:%@#HF7@#LRI@#P_[@#Tkn@#Xw*A#]-=A#a9OA#"
    "d<F&#*;G##.GY##2Sl##6`($#:l:$#>xL$#B.`$#F:r$#JF.%#NR@%#R_R%#Vke%#Zww%#_-4&#3^Rh%Sflr-k'MS.o?.5/sWel/wpEM0%3'/1)K^f1-d>G21&v(35>V`39V7A4=onx4"
    "A1OY5EI0;6Ibgr6M$HS7Q<)58C5w,;WoA*#[%T*#`1g*#d=#+#hI5+#lUG+#pbY+#tnl+#x$),#&1;,#*=M,#.I`,#2Ur,#6b.-#;w[H#iQtA#m^0B#qjBB#uvTB##-hB#'9$C#+E6C#"
    "/QHC#3^ZC#7jmC#;v)D#?,<D#C8ND#GDaD#KPsD#O]/E#g1A5#KA*1#gC17#MGd;#8(02#L-d3#rWM4#Hga1#,<w0#T.j<#O#'2#CYN1#qa^:#_4m3#o@/=#eG8=#t8J5#`+78#4uI-#"
    "m3B2#SB[8#Q0@8#i[*9#iOn8#1Nm;#^sN9#qh<9#:=x-#P;K2#$%X9#bC+.#Rg;<#mN=.#MTF.#RZO.#2?)4#Y#(/#[)1/#b;L/#dAU/#0Sv;#lY$0#n`-0#sf60#(F24#wrH0#%/e0#"
    "TmD<#%JSMFove:CTBEXI:<eh2g)B,3h2^G3i;#d3jD>)4kMYD4lVu`4m`:&5niUA5@(A5BA1]PBB:xlBCC=2CDLXMCEUtiCf&0g2'tN?PGT4CPGT4CPGT4CPGT4CPGT4CPGT4CPGT4CP"
    "GT4CPGT4CPGT4CPGT4CPGT4CPGT4CP-qekC`.9kEg^+F$kwViFJTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5o,^<-28ZI'O?;xp"
    "O?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xp;7q-#lLYI:xvD=#";

#endif /* NK_INCLUDE_DEFAULT_FONT */

#define NK_CURSOR_DATA_W 90
#define NK_CURSOR_DATA_H 27
NK_GLOBAL const char nk_custom_cursor_data[NK_CURSOR_DATA_W * NK_CURSOR_DATA_H + 1] =
{
    "..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX"
    "..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X"
    "---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X"
    "X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X"
    "XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X"
    "X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X"
    "X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX"
    "X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      "
    "X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       "
    "X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        "
    "X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         "
    "X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          "
    "X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           "
    "X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            "
    "X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           "
    "X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          "
    "X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          "
    "X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       ------------------------------------"
    "X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           "
    "XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           "
    "      X..X          -  X...X  -         X...X         -  X..X           X..X  -           "
    "       XX           -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           "
    "------------        -    X    -           X           -X.....................X-           "
    "                    ----------------------------------- X...XXXXXXXXXXXXX...X -           "
    "                                                      -  X..X           X..X  -           "
    "                                                      -   X.X           X.X   -           "
    "                                                      -    XX           XX    -           "
};

#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

NK_GLOBAL unsigned char *nk__barrier;
NK_GLOBAL unsigned char *nk__barrier2;
NK_GLOBAL unsigned char *nk__barrier3;
NK_GLOBAL unsigned char *nk__barrier4;
NK_GLOBAL unsigned char *nk__dout;

NK_INTERN unsigned int
nk_decompress_length(unsigned char *input)
{
    return (unsigned int)((input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11]);
}
NK_INTERN void
nk__match(unsigned char *data, unsigned int length)
{
    /* INVERSE of memmove... write each byte before copying the next...*/
    NK_ASSERT (nk__dout + length <= nk__barrier);
    if (nk__dout + length > nk__barrier) { nk__dout += length; return; }
    if (data < nk__barrier4) { nk__dout = nk__barrier+1; return; }
    while (length--) *nk__dout++ = *data++;
}
NK_INTERN void
nk__lit(unsigned char *data, unsigned int length)
{
    NK_ASSERT (nk__dout + length <= nk__barrier);
    if (nk__dout + length > nk__barrier) { nk__dout += length; return; }
    if (data < nk__barrier2) { nk__dout = nk__barrier+1; return; }
    NK_MEMCPY(nk__dout, data, length);
    nk__dout += length;
}
NK_INTERN unsigned char*
nk_decompress_token(unsigned char *i)
{
    #define nk__in2(x)   ((i[x] << 8) + i[(x)+1])
    #define nk__in3(x)   ((i[x] << 16) + nk__in2((x)+1))
    #define nk__in4(x)   ((i[x] << 24) + nk__in3((x)+1))

    if (*i >= 0x20) { /* use fewer if's for cases that expand small */
        if (*i >= 0x80)       nk__match(nk__dout-i[1]-1, (unsigned int)i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)  nk__match(nk__dout-(nk__in2(0) - 0x4000 + 1), (unsigned int)i[2]+1), i += 3;
        else /* *i >= 0x20 */ nk__lit(i+1, (unsigned int)i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    } else { /* more ifs for cases that expand large, since overhead is amortized */
        if (*i >= 0x18)       nk__match(nk__dout-(unsigned int)(nk__in3(0) - 0x180000 + 1), (unsigned int)i[3]+1), i += 4;
        else if (*i >= 0x10)  nk__match(nk__dout-(unsigned int)(nk__in3(0) - 0x100000 + 1), (unsigned int)nk__in2(3)+1), i += 5;
        else if (*i >= 0x08)  nk__lit(i+2, (unsigned int)nk__in2(0) - 0x0800 + 1), i += 2 + (nk__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)  nk__lit(i+3, (unsigned int)nk__in2(1) + 1), i += 3 + (nk__in2(1) + 1);
        else if (*i == 0x06)  nk__match(nk__dout-(unsigned int)(nk__in3(1)+1), i[4]+1u), i += 5;
        else if (*i == 0x04)  nk__match(nk__dout-(unsigned int)(nk__in3(1)+1), (unsigned int)nk__in2(4)+1u), i += 6;
    }
    return i;
}
NK_INTERN unsigned int
nk_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen)
{
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen, i;

    blocklen = buflen % 5552;
    while (buflen) {
        for (i=0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0]; s2 += s1;
            s1 += buffer[1]; s2 += s1;
            s1 += buffer[2]; s2 += s1;
            s1 += buffer[3]; s2 += s1;
            s1 += buffer[4]; s2 += s1;
            s1 += buffer[5]; s2 += s1;
            s1 += buffer[6]; s2 += s1;
            s1 += buffer[7]; s2 += s1;
            buffer += 8;
        }
        for (; i < blocklen; ++i) {
            s1 += *buffer++; s2 += s1;
        }

        s1 %= ADLER_MOD; s2 %= ADLER_MOD;
        buflen -= (unsigned int)blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}
NK_INTERN unsigned int
nk_decompress(unsigned char *output, unsigned char *i, unsigned int length)
{
    unsigned int olen;
    if (nk__in4(0) != 0x57bC0000) return 0;
    if (nk__in4(4) != 0)          return 0; /* error! stream is > 4GB */
    olen = nk_decompress_length(i);
    nk__barrier2 = i;
    nk__barrier3 = i+length;
    nk__barrier = output + olen;
    nk__barrier4 = output;
    i += 16;

    nk__dout = output;
    for (;;) {
        unsigned char *old_i = i;
        i = nk_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                NK_ASSERT(nk__dout == output + olen);
                if (nk__dout != output + olen) return 0;
                if (nk_adler32(1, output, olen) != (unsigned int) nk__in4(2))
                    return 0;
                return olen;
            } else {
                NK_ASSERT(0); /* NOTREACHED */
                return 0;
            }
        }
        NK_ASSERT(nk__dout <= output + olen);
        if (nk__dout > output + olen)
            return 0;
    }
}
NK_INTERN unsigned int
nk_decode_85_byte(char c)
{
    return (unsigned int)((c >= '\\') ? c-36 : c-35);
}
NK_INTERN void
nk_decode_85(unsigned char* dst, const unsigned char* src)
{
    while (*src)
    {
        unsigned int tmp =
            nk_decode_85_byte((char)src[0]) +
            85 * (nk_decode_85_byte((char)src[1]) +
            85 * (nk_decode_85_byte((char)src[2]) +
            85 * (nk_decode_85_byte((char)src[3]) +
            85 * nk_decode_85_byte((char)src[4]))));

        /* we can't assume little-endianess. */
        dst[0] = (unsigned char)((tmp >> 0) & 0xFF);
        dst[1] = (unsigned char)((tmp >> 8) & 0xFF);
        dst[2] = (unsigned char)((tmp >> 16) & 0xFF);
        dst[3] = (unsigned char)((tmp >> 24) & 0xFF);

        src += 5;
        dst += 4;
    }
}

/* -------------------------------------------------------------
 *
 *                          FONT ATLAS
 *
 * --------------------------------------------------------------*/
NK_API struct nk_font_config
nk_font_config(float pixel_height)
{
    struct nk_font_config cfg;
    nk_zero_struct(cfg);
    cfg.ttf_blob = 0;
    cfg.ttf_size = 0;
    cfg.ttf_data_owned_by_atlas = 0;
    cfg.size = pixel_height;
    cfg.oversample_h = 3;
    cfg.oversample_v = 1;
    cfg.pixel_snap = 0;
    cfg.coord_type = NK_COORD_UV;
    cfg.spacing = nk_vec2(0,0);
    cfg.range = nk_font_default_glyph_ranges();
    cfg.merge_mode = 0;
    cfg.fallback_glyph = '?';
    cfg.font = 0;
    cfg.n = 0;
    return cfg;
}
#ifdef NK_INCLUDE_DEFAULT_ALLOCATOR
NK_API void
nk_font_atlas_init_default(struct nk_font_atlas *atlas)
{
    NK_ASSERT(atlas);
    if (!atlas) return;
    nk_zero_struct(*atlas);
    atlas->temporary.userdata.ptr = 0;
    atlas->temporary.alloc = nk_malloc;
    atlas->temporary.free = nk_mfree;
    atlas->permanent.userdata.ptr = 0;
    atlas->permanent.alloc = nk_malloc;
    atlas->permanent.free = nk_mfree;
}
#endif
NK_API void
nk_font_atlas_init(struct nk_font_atlas *atlas, struct nk_allocator *alloc)
{
    NK_ASSERT(atlas);
    NK_ASSERT(alloc);
    if (!atlas || !alloc) return;
    nk_zero_struct(*atlas);
    atlas->permanent = *alloc;
    atlas->temporary = *alloc;
}
NK_API void
nk_font_atlas_init_custom(struct nk_font_atlas *atlas,
    struct nk_allocator *permanent, struct nk_allocator *temporary)
{
    NK_ASSERT(atlas);
    NK_ASSERT(permanent);
    NK_ASSERT(temporary);
    if (!atlas || !permanent || !temporary) return;
    nk_zero_struct(*atlas);
    atlas->permanent = *permanent;
    atlas->temporary = *temporary;
}
NK_API void
nk_font_atlas_begin(struct nk_font_atlas *atlas)
{
    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc && atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc && atlas->permanent.free);
    if (!atlas || !atlas->permanent.alloc || !atlas->permanent.free ||
        !atlas->temporary.alloc || !atlas->temporary.free) return;
    if (atlas->glyphs) {
        atlas->permanent.free(atlas->permanent.userdata, atlas->glyphs);
        atlas->glyphs = 0;
    }
    if (atlas->pixel) {
        atlas->permanent.free(atlas->permanent.userdata, atlas->pixel);
        atlas->pixel = 0;
    }
}
NK_API struct nk_font*
nk_font_atlas_add(struct nk_font_atlas *atlas, const struct nk_font_config *config)
{
    struct nk_font *font = 0;
    struct nk_font_config *cfg;

    NK_ASSERT(atlas);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);

    NK_ASSERT(config);
    NK_ASSERT(config->ttf_blob);
    NK_ASSERT(config->ttf_size);
    NK_ASSERT(config->size > 0.0f);

    if (!atlas || !config || !config->ttf_blob || !config->ttf_size || config->size <= 0.0f||
        !atlas->permanent.alloc || !atlas->permanent.free ||
        !atlas->temporary.alloc || !atlas->temporary.free)
        return 0;

    /* allocate font config  */
    cfg = (struct nk_font_config*)
        atlas->permanent.alloc(atlas->permanent.userdata,0, sizeof(struct nk_font_config));
    NK_MEMCPY(cfg, config, sizeof(*config));
    cfg->n = cfg;
    cfg->p = cfg;

    if (!config->merge_mode) {
        /* insert font config into list */
        if (!atlas->config) {
            atlas->config = cfg;
            cfg->next = 0;
        } else {
            struct nk_font_config *i = atlas->config;
            while (i->next) i = i->next;
            i->next = cfg;
            cfg->next = 0;
        }
        /* allocate new font */
        font = (struct nk_font*)
            atlas->permanent.alloc(atlas->permanent.userdata,0, sizeof(struct nk_font));
        NK_ASSERT(font);
        nk_zero(font, sizeof(*font));
        if (!font) return 0;
        font->config = cfg;

        /* insert font into list */
        if (!atlas->fonts) {
            atlas->fonts = font;
            font->next = 0;
        } else {
            struct nk_font *i = atlas->fonts;
            while (i->next) i = i->next;
            i->next = font;
            font->next = 0;
        }
        cfg->font = &font->info;
    } else {
        /* extend previously added font */
        struct nk_font *f = 0;
        struct nk_font_config *c = 0;
        NK_ASSERT(atlas->font_num);
        f = atlas->fonts;
        c = f->config;
        cfg->font = &f->info;

        cfg->n = c;
        cfg->p = c->p;
        c->p->n = cfg;
        c->p = cfg;
    }
    /* create own copy of .TTF font blob */
    if (!config->ttf_data_owned_by_atlas) {
        cfg->ttf_blob = atlas->permanent.alloc(atlas->permanent.userdata,0, cfg->ttf_size);
        NK_ASSERT(cfg->ttf_blob);
        if (!cfg->ttf_blob) {
            atlas->font_num++;
            return 0;
        }
        NK_MEMCPY(cfg->ttf_blob, config->ttf_blob, cfg->ttf_size);
        cfg->ttf_data_owned_by_atlas = 1;
    }
    atlas->font_num++;
    return font;
}
NK_API struct nk_font*
nk_font_atlas_add_from_memory(struct nk_font_atlas *atlas, void *memory,
    nk_size size, float height, const struct nk_font_config *config)
{
    struct nk_font_config cfg;
    NK_ASSERT(memory);
    NK_ASSERT(size);

    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);
    if (!atlas || !atlas->temporary.alloc || !atlas->temporary.free || !memory || !size ||
        !atlas->permanent.alloc || !atlas->permanent.free)
        return 0;

    cfg = (config) ? *config: nk_font_config(height);
    cfg.ttf_blob = memory;
    cfg.ttf_size = size;
    cfg.size = height;
    cfg.ttf_data_owned_by_atlas = 0;
    return nk_font_atlas_add(atlas, &cfg);
}
#ifdef NK_INCLUDE_STANDARD_IO
NK_API struct nk_font*
nk_font_atlas_add_from_file(struct nk_font_atlas *atlas, const char *file_path,
    float height, const struct nk_font_config *config)
{
    nk_size size;
    char *memory;
    struct nk_font_config cfg;

    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);

    if (!atlas || !file_path) return 0;
    memory = nk_file_load(file_path, &size, &atlas->permanent);
    if (!memory) return 0;

    cfg = (config) ? *config: nk_font_config(height);
    cfg.ttf_blob = memory;
    cfg.ttf_size = size;
    cfg.size = height;
    cfg.ttf_data_owned_by_atlas = 1;
    return nk_font_atlas_add(atlas, &cfg);
}
#endif
NK_API struct nk_font*
nk_font_atlas_add_compressed(struct nk_font_atlas *atlas,
    void *compressed_data, nk_size compressed_size, float height,
    const struct nk_font_config *config)
{
    unsigned int decompressed_size;
    void *decompressed_data;
    struct nk_font_config cfg;

    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);

    NK_ASSERT(compressed_data);
    NK_ASSERT(compressed_size);
    if (!atlas || !compressed_data || !atlas->temporary.alloc || !atlas->temporary.free ||
        !atlas->permanent.alloc || !atlas->permanent.free)
        return 0;

    decompressed_size = nk_decompress_length((unsigned char*)compressed_data);
    decompressed_data = atlas->permanent.alloc(atlas->permanent.userdata,0,decompressed_size);
    NK_ASSERT(decompressed_data);
    if (!decompressed_data) return 0;
    nk_decompress((unsigned char*)decompressed_data, (unsigned char*)compressed_data,
        (unsigned int)compressed_size);

    cfg = (config) ? *config: nk_font_config(height);
    cfg.ttf_blob = decompressed_data;
    cfg.ttf_size = decompressed_size;
    cfg.size = height;
    cfg.ttf_data_owned_by_atlas = 1;
    return nk_font_atlas_add(atlas, &cfg);
}
NK_API struct nk_font*
nk_font_atlas_add_compressed_base85(struct nk_font_atlas *atlas,
    const char *data_base85, float height, const struct nk_font_config *config)
{
    int compressed_size;
    void *compressed_data;
    struct nk_font *font;

    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);

    NK_ASSERT(data_base85);
    if (!atlas || !data_base85 || !atlas->temporary.alloc || !atlas->temporary.free ||
        !atlas->permanent.alloc || !atlas->permanent.free)
        return 0;

    compressed_size = (((int)nk_strlen(data_base85) + 4) / 5) * 4;
    compressed_data = atlas->temporary.alloc(atlas->temporary.userdata,0, (nk_size)compressed_size);
    NK_ASSERT(compressed_data);
    if (!compressed_data) return 0;
    nk_decode_85((unsigned char*)compressed_data, (const unsigned char*)data_base85);
    font = nk_font_atlas_add_compressed(atlas, compressed_data,
                    (nk_size)compressed_size, height, config);
    atlas->temporary.free(atlas->temporary.userdata, compressed_data);
    return font;
}

#ifdef NK_INCLUDE_DEFAULT_FONT
NK_API struct nk_font*
nk_font_atlas_add_default(struct nk_font_atlas *atlas,
    float pixel_height, const struct nk_font_config *config)
{
    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);
    return nk_font_atlas_add_compressed_base85(atlas,
        nk_proggy_clean_ttf_compressed_data_base85, pixel_height, config);
}
#endif
NK_API const void*
nk_font_atlas_bake(struct nk_font_atlas *atlas, int *width, int *height,
    enum nk_font_atlas_format fmt)
{
    int i = 0;
    void *tmp = 0;
    nk_size tmp_size, img_size;
    struct nk_font *font_iter;
    struct nk_font_baker *baker;

    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);

    NK_ASSERT(width);
    NK_ASSERT(height);
    if (!atlas || !width || !height ||
        !atlas->temporary.alloc || !atlas->temporary.free ||
        !atlas->permanent.alloc || !atlas->permanent.free)
        return 0;

#ifdef NK_INCLUDE_DEFAULT_FONT
    /* no font added so just use default font */
    if (!atlas->font_num)
        atlas->default_font = nk_font_atlas_add_default(atlas, 13.0f, 0);
#endif
    NK_ASSERT(atlas->font_num);
    if (!atlas->font_num) return 0;

    /* allocate temporary baker memory required for the baking process */
    nk_font_baker_memory(&tmp_size, &atlas->glyph_count, atlas->config, atlas->font_num);
    tmp = atlas->temporary.alloc(atlas->temporary.userdata,0, tmp_size);
    NK_ASSERT(tmp);
    if (!tmp) goto failed;

    /* allocate glyph memory for all fonts */
    baker = nk_font_baker(tmp, atlas->glyph_count, atlas->font_num, &atlas->temporary);
    atlas->glyphs = (struct nk_font_glyph*)atlas->permanent.alloc(
        atlas->permanent.userdata,0, sizeof(struct nk_font_glyph)*(nk_size)atlas->glyph_count);
    NK_ASSERT(atlas->glyphs);
    if (!atlas->glyphs)
        goto failed;

    /* pack all glyphs into a tight fit space */
    atlas->custom.w = (NK_CURSOR_DATA_W*2)+1;
    atlas->custom.h = NK_CURSOR_DATA_H + 1;
    if (!nk_font_bake_pack(baker, &img_size, width, height, &atlas->custom,
        atlas->config, atlas->font_num, &atlas->temporary))
        goto failed;

    /* allocate memory for the baked image font atlas */
    atlas->pixel = atlas->temporary.alloc(atlas->temporary.userdata,0, img_size);
    NK_ASSERT(atlas->pixel);
    if (!atlas->pixel)
        goto failed;

    /* bake glyphs and custom white pixel into image */
    nk_font_bake(baker, atlas->pixel, *width, *height,
        atlas->glyphs, atlas->glyph_count, atlas->config, atlas->font_num);
    nk_font_bake_custom_data(atlas->pixel, *width, *height, atlas->custom,
            nk_custom_cursor_data, NK_CURSOR_DATA_W, NK_CURSOR_DATA_H, '.', 'X');

    if (fmt == NK_FONT_ATLAS_RGBA32) {
        /* convert alpha8 image into rgba32 image */
        void *img_rgba = atlas->temporary.alloc(atlas->temporary.userdata,0,
                            (nk_size)(*width * *height * 4));
        NK_ASSERT(img_rgba);
        if (!img_rgba) goto failed;
        nk_font_bake_convert(img_rgba, *width, *height, atlas->pixel);
        atlas->temporary.free(atlas->temporary.userdata, atlas->pixel);
        atlas->pixel = img_rgba;
    }
    atlas->tex_width = *width;
    atlas->tex_height = *height;

    /* initialize each font */
    for (font_iter = atlas->fonts; font_iter; font_iter = font_iter->next) {
        struct nk_font *font = font_iter;
        struct nk_font_config *config = font->config;
        nk_font_init(font, config->size, config->fallback_glyph, atlas->glyphs,
            config->font, nk_handle_ptr(0));
    }

    /* initialize each cursor */
    {NK_STORAGE const struct nk_vec2 nk_cursor_data[NK_CURSOR_COUNT][3] = {
        /* Pos      Size        Offset */
        {{ 0, 3},   {12,19},    { 0, 0}},
        {{13, 0},   { 7,16},    { 4, 8}},
        {{31, 0},   {23,23},    {11,11}},
        {{21, 0},   { 9, 23},   { 5,11}},
        {{55,18},   {23, 9},    {11, 5}},
        {{73, 0},   {17,17},    { 9, 9}},
        {{55, 0},   {17,17},    { 9, 9}}
    };
    for (i = 0; i < NK_CURSOR_COUNT; ++i) {
        struct nk_cursor *cursor = &atlas->cursors[i];
        cursor->img.w = (unsigned short)*width;
        cursor->img.h = (unsigned short)*height;
        cursor->img.region[0] = (unsigned short)(atlas->custom.x + nk_cursor_data[i][0].x);
        cursor->img.region[1] = (unsigned short)(atlas->custom.y + nk_cursor_data[i][0].y);
        cursor->img.region[2] = (unsigned short)nk_cursor_data[i][1].x;
        cursor->img.region[3] = (unsigned short)nk_cursor_data[i][1].y;
        cursor->size = nk_cursor_data[i][1];
        cursor->offset = nk_cursor_data[i][2];
    }}
    /* free temporary memory */
    atlas->temporary.free(atlas->temporary.userdata, tmp);
    return atlas->pixel;

failed:
    /* error so cleanup all memory */
    if (tmp) atlas->temporary.free(atlas->temporary.userdata, tmp);
    if (atlas->glyphs) {
        atlas->permanent.free(atlas->permanent.userdata, atlas->glyphs);
        atlas->glyphs = 0;
    }
    if (atlas->pixel) {
        atlas->temporary.free(atlas->temporary.userdata, atlas->pixel);
        atlas->pixel = 0;
    }
    return 0;
}
NK_API void
nk_font_atlas_end(struct nk_font_atlas *atlas, nk_handle texture,
    struct nk_draw_null_texture *null)
{
    int i = 0;
    struct nk_font *font_iter;
    NK_ASSERT(atlas);
    if (!atlas) {
        if (!null) return;
        null->texture = texture;
        null->uv = nk_vec2(0.5f,0.5f);
    }
    if (null) {
        null->texture = texture;
        null->uv.x = (atlas->custom.x + 0.5f)/(float)atlas->tex_width;
        null->uv.y = (atlas->custom.y + 0.5f)/(float)atlas->tex_height;
    }
    for (font_iter = atlas->fonts; font_iter; font_iter = font_iter->next) {
        font_iter->texture = texture;
#ifdef NK_INCLUDE_VERTEX_BUFFER_OUTPUT
        font_iter->handle.texture = texture;
#endif
    }
    for (i = 0; i < NK_CURSOR_COUNT; ++i)
        atlas->cursors[i].img.handle = texture;

    atlas->temporary.free(atlas->temporary.userdata, atlas->pixel);
    atlas->pixel = 0;
    atlas->tex_width = 0;
    atlas->tex_height = 0;
    atlas->custom.x = 0;
    atlas->custom.y = 0;
    atlas->custom.w = 0;
    atlas->custom.h = 0;
}
NK_API void
nk_font_atlas_cleanup(struct nk_font_atlas *atlas)
{
    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);
    if (!atlas || !atlas->permanent.alloc || !atlas->permanent.free) return;
    if (atlas->config) {
        struct nk_font_config *iter;
        for (iter = atlas->config; iter; iter = iter->next) {
            struct nk_font_config *i;
            for (i = iter->n; i != iter; i = i->n) {
                atlas->permanent.free(atlas->permanent.userdata, i->ttf_blob);
                i->ttf_blob = 0;
            }
            atlas->permanent.free(atlas->permanent.userdata, iter->ttf_blob);
            iter->ttf_blob = 0;
        }
    }
}
NK_API void
nk_font_atlas_clear(struct nk_font_atlas *atlas)
{
    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);
    if (!atlas || !atlas->permanent.alloc || !atlas->permanent.free) return;

    if (atlas->config) {
        struct nk_font_config *iter, *next;
        for (iter = atlas->config; iter; iter = next) {
            struct nk_font_config *i, *n;
            for (i = iter->n; i != iter; i = n) {
                n = i->n;
                if (i->ttf_blob)
                    atlas->permanent.free(atlas->permanent.userdata, i->ttf_blob);
                atlas->permanent.free(atlas->permanent.userdata, i);
            }
            next = iter->next;
            if (i->ttf_blob)
                atlas->permanent.free(atlas->permanent.userdata, iter->ttf_blob);
            atlas->permanent.free(atlas->permanent.userdata, iter);
        }
        atlas->config = 0;
    }
    if (atlas->fonts) {
        struct nk_font *iter, *next;
        for (iter = atlas->fonts; iter; iter = next) {
            next = iter->next;
            atlas->permanent.free(atlas->permanent.userdata, iter);
        }
        atlas->fonts = 0;
    }
    if (atlas->glyphs)
        atlas->permanent.free(atlas->permanent.userdata, atlas->glyphs);
    nk_zero_struct(*atlas);
}
#endif

