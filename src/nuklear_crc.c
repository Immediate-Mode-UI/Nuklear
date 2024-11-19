/** 
 * \file nuklear_crc.c 
 * \brief Implements CRC checking of the draw command buffer.
 * 
 * \details Implements a rolling CRC that gets updated with every draw_push
 * function call. That is, for every `nk_xxx` api function call between
 * `nk_begin()` and `nk_end()` there is one to many draw commands pushed into
 * the draw buffer.  For each one of those pushes, the command type is XOR'd
 * then stored into that buffer's CRC member. Then on the next `nk_xxx` function
 * call, another command push is executed and the previous crc is updated with
 * the new XOR of the new command. This will result in a unique CRC value in the
 * buffers crc member at the end of the draw execution. 
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
 * Yet implementing this CRC, we cruch all of that down into just the initial
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
 * If you want to use the CRC, then you must either `#define NK_DRAW_BUFFER_CRC`
 * or `#define NK_DRAW_BUFFER_CRC_CUSTOM` 
 *
 * `#define NK_DRAW_BUFFER_CRC` will implement the default CRC algorithm which is
 * a 32 bit castagnoli (CRC-32C). This is a very common CRC that is used in
 * iSCSI, SMPTE, ext-4, etc. So common that intel processors have dedicated
 * hardware for it and lots of libraries have SIMD acceleration for it (not that
 * its necessary for this application) 
 *
 * `#define NK_DRAW_BUFFER_CRC_CUSTOM` will allow the user to define their own CRC
 * implementation. This is available because most embedded systems already have
 * a CRC calculation of some kind, usually defined by their system constraints,
 * so it would be redundant to create yet another CRC method. 
 *
 * To use the `NK_DRAW_BUFFER_CRC_CUSTOM` the user will need to `#define
 * NK_CRC_SEED` to their desired seed and also `#define NK_CRC_FUNC(c,d,l)` to
 * point to a user defined implementation of the crc.
 *
 * The user defined implementation of the CRC **SHALL** have the declaration of
 * ```c
 * NK_UINT32 <xxCustomNamexx>(NK_UINT32 crc, NK_UINT8 *data, NK_SIZE_TYPE len)
 * ```
 * That is, Shall :
 * - return a 32 bit integer (the new CRC value)
 * - accept, as the first argument, a 32 bit integer. The initial value of the crc.
 * - accept, as the second argument, a pointer to a byte buffer to be CRC'd.
 * - accept, as the third argument, the length of the byte buffer.
 *
 * other than that, the user is free to use any CRC algorithm, table,
 * implementation they choose.
 *
 * \addtogroup crc 
 * \brief Command buffer CRC Implementation
 * @{
 */

#ifdef NK_DRAW_BUFFER_CRC_CUSTOM
#ifndef NK_CRC_SEED
#error "Must #define NK_CRC_SEED when using NK_DRAW_BUFFER_CRC_CUSTOM"
#endif
#ifndef NK_CRC_FUNC(b,d,l)
#error "Must #define NK_CRC_FUNC(c,d,l) when using NK_DRAW_BUFFER_CRC_CUSTOM"
#endif
#endif

#ifdef NK_DRAW_BUFFER_CRC
#define NK_CRC_SEED 0xffffffff /**< seed value of the crc*/
#define NK_CRC_FUNC(c,d,l) nk_crc_update(c, d, l)
#include "nuklear.h"
#include "nuklear_internal.h"

NK_STORAGE NK_UINT32 crc32c_table[] = {
 0x00000000L, 0xF26B8303L, 0xE13B70F7L, 0x1350F3F4L, 0xC79A971FL, 0x35F1141CL,
 0x26A1E7E8L, 0xD4CA64EBL, 0x8AD958CFL, 0x78B2DBCCL, 0x6BE22838L, 0x9989AB3BL,
 0x4D43CFD0L, 0xBF284CD3L, 0xAC78BF27L, 0x5E133C24L, 0x105EC76FL, 0xE235446CL,
 0xF165B798L, 0x030E349BL, 0xD7C45070L, 0x25AFD373L, 0x36FF2087L, 0xC494A384L,
 0x9A879FA0L, 0x68EC1CA3L, 0x7BBCEF57L, 0x89D76C54L, 0x5D1D08BFL, 0xAF768BBCL,
 0xBC267848L, 0x4E4DFB4BL, 0x20BD8EDEL, 0xD2D60DDDL, 0xC186FE29L, 0x33ED7D2AL,
 0xE72719C1L, 0x154C9AC2L, 0x061C6936L, 0xF477EA35L, 0xAA64D611L, 0x580F5512L,
 0x4B5FA6E6L, 0xB93425E5L, 0x6DFE410EL, 0x9F95C20DL, 0x8CC531F9L, 0x7EAEB2FAL,
 0x30E349B1L, 0xC288CAB2L, 0xD1D83946L, 0x23B3BA45L, 0xF779DEAEL, 0x05125DADL,
 0x1642AE59L, 0xE4292D5AL, 0xBA3A117EL, 0x4851927DL, 0x5B016189L, 0xA96AE28AL,
 0x7DA08661L, 0x8FCB0562L, 0x9C9BF696L, 0x6EF07595L, 0x417B1DBCL, 0xB3109EBFL,
 0xA0406D4BL, 0x522BEE48L, 0x86E18AA3L, 0x748A09A0L, 0x67DAFA54L, 0x95B17957L,
 0xCBA24573L, 0x39C9C670L, 0x2A993584L, 0xD8F2B687L, 0x0C38D26CL, 0xFE53516FL,
 0xED03A29BL, 0x1F682198L, 0x5125DAD3L, 0xA34E59D0L, 0xB01EAA24L, 0x42752927L,
 0x96BF4DCCL, 0x64D4CECFL, 0x77843D3BL, 0x85EFBE38L, 0xDBFC821CL, 0x2997011FL,
 0x3AC7F2EBL, 0xC8AC71E8L, 0x1C661503L, 0xEE0D9600L, 0xFD5D65F4L, 0x0F36E6F7L,
 0x61C69362L, 0x93AD1061L, 0x80FDE395L, 0x72966096L, 0xA65C047DL, 0x5437877EL,
 0x4767748AL, 0xB50CF789L, 0xEB1FCBADL, 0x197448AEL, 0x0A24BB5AL, 0xF84F3859L,
 0x2C855CB2L, 0xDEEEDFB1L, 0xCDBE2C45L, 0x3FD5AF46L, 0x7198540DL, 0x83F3D70EL,
 0x90A324FAL, 0x62C8A7F9L, 0xB602C312L, 0x44694011L, 0x5739B3E5L, 0xA55230E6L,
 0xFB410CC2L, 0x092A8FC1L, 0x1A7A7C35L, 0xE811FF36L, 0x3CDB9BDDL, 0xCEB018DEL,
 0xDDE0EB2AL, 0x2F8B6829L, 0x82F63B78L, 0x709DB87BL, 0x63CD4B8FL, 0x91A6C88CL,
 0x456CAC67L, 0xB7072F64L, 0xA457DC90L, 0x563C5F93L, 0x082F63B7L, 0xFA44E0B4L,
 0xE9141340L, 0x1B7F9043L, 0xCFB5F4A8L, 0x3DDE77ABL, 0x2E8E845FL, 0xDCE5075CL,
 0x92A8FC17L, 0x60C37F14L, 0x73938CE0L, 0x81F80FE3L, 0x55326B08L, 0xA759E80BL,
 0xB4091BFFL, 0x466298FCL, 0x1871A4D8L, 0xEA1A27DBL, 0xF94AD42FL, 0x0B21572CL,
 0xDFEB33C7L, 0x2D80B0C4L, 0x3ED04330L, 0xCCBBC033L, 0xA24BB5A6L, 0x502036A5L,
 0x4370C551L, 0xB11B4652L, 0x65D122B9L, 0x97BAA1BAL, 0x84EA524EL, 0x7681D14DL,
 0x2892ED69L, 0xDAF96E6AL, 0xC9A99D9EL, 0x3BC21E9DL, 0xEF087A76L, 0x1D63F975L,
 0x0E330A81L, 0xFC588982L, 0xB21572C9L, 0x407EF1CAL, 0x532E023EL, 0xA145813DL,
 0x758FE5D6L, 0x87E466D5L, 0x94B49521L, 0x66DF1622L, 0x38CC2A06L, 0xCAA7A905L,
 0xD9F75AF1L, 0x2B9CD9F2L, 0xFF56BD19L, 0x0D3D3E1AL, 0x1E6DCDEEL, 0xEC064EEDL,
 0xC38D26C4L, 0x31E6A5C7L, 0x22B65633L, 0xD0DDD530L, 0x0417B1DBL, 0xF67C32D8L,
 0xE52CC12CL, 0x1747422FL, 0x49547E0BL, 0xBB3FFD08L, 0xA86F0EFCL, 0x5A048DFFL,
 0x8ECEE914L, 0x7CA56A17L, 0x6FF599E3L, 0x9D9E1AE0L, 0xD3D3E1ABL, 0x21B862A8L,
 0x32E8915CL, 0xC083125FL, 0x144976B4L, 0xE622F5B7L, 0xF5720643L, 0x07198540L,
 0x590AB964L, 0xAB613A67L, 0xB831C993L, 0x4A5A4A90L, 0x9E902E7BL, 0x6CFBAD78L,
 0x7FAB5E8CL, 0x8DC0DD8FL, 0xE330A81AL, 0x115B2B19L, 0x020BD8EDL, 0xF0605BEEL,
 0x24AA3F05L, 0xD6C1BC06L, 0xC5914FF2L, 0x37FACCF1L, 0x69E9F0D5L, 0x9B8273D6L,
 0x88D28022L, 0x7AB90321L, 0xAE7367CAL, 0x5C18E4C9L, 0x4F48173DL, 0xBD23943EL,
 0xF36E6F75L, 0x0105EC76L, 0x12551F82L, 0xE03E9C81L, 0x34F4F86AL, 0xC69F7B69L,
 0xD5CF889DL, 0x27A40B9EL, 0x79B737BAL, 0x8BDCB4B9L, 0x988C474DL, 0x6AE7C44EL,
 0xBE2DA0A5L, 0x4C4623A6L, 0x5F16D052L, 0xAD7D5351L
};



/**
 * \brief steps the crc value by the amount of new data.
 *
 * \details
 * this function should only be called in the nk_command_buffer_push() function
 * and should only be given the `type` enum of the command being pushed. As the
 * buffer is getting filled, this function will roll and calculate the CRC
 * value. Never clearing will result is a proper CRC at the end of the drawing
 * updates.
 *
 * \param[in] crc is the previous value of the CRC from which to build on.
 * \param[in] data is a pointer to the data to run the CRC on.
 * \param[in] len is the size in bytes of the data.
 *
 * \returns the new crc value 
 */
NK_LIB NK_UINT32 nk_crc_update(NK_UINT32 crc, NK_UINT8 *data, NK_SIZE_TYPE len)
{
   while (len--) crc = (crc<<8) ^ crc32c_table[(crc >> 24) ^ *data++];
   return crc;
}
#endif

#if defined(NK_DRAW_BUFFER_CRC) || defined(NK_DRAW_BUFFER_CRC_CUSTOM)
/**
 * \brief re-initializes the command buffer crc.
 *
 * \details
 * should be called on nk_clear such that the CRC can start over.
 *
 * \param[in] buf is the command buffer for which crc to clear
 */
NK_LIB void nk_crc_clear(struct nk_command_buffer *buf)
{
    buf->crc = NK_CRC_SEED;
}

/**
 * \brief returns the crc of the command buffer.
 * \param[in] buf is the command buffer for which crc to clear
 */
NK_API NK_UINT32 nk_buffer_crc(struct nk_context *ctx)
{
    return ctx->overlay.crc;
}
/** @} *//*end documentation grouping*/
#endif
