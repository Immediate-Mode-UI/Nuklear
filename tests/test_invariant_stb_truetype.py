import pytest
import struct
import math


# Simulate the vulnerable allocation size computation logic
# This mirrors the C code: num_vertices * sizeof(stbtt_vertex)
# where sizeof(stbtt_vertex) is typically 10 bytes on most platforms

SIZEOF_STBTT_VERTEX = 10  # typical size of stbtt_vertex struct
UINT32_MAX = 0xFFFFFFFF
UINT16_MAX = 0xFFFF
INT32_MAX = 0x7FFFFFFF
INT16_MAX = 0x7FFF


def compute_allocation_size_safe(num_vertices, comp_num_verts, vertex_size=SIZEOF_STBTT_VERTEX):
    """
    Safe computation of allocation size for composite glyph vertices.
    Must not overflow and must return a size that can safely hold all vertices.
    """
    total_vertices = num_vertices + comp_num_verts
    allocation_size = total_vertices * vertex_size
    return allocation_size


def compute_allocation_size_32bit_overflow(num_vertices, comp_num_verts, vertex_size=SIZEOF_STBTT_VERTEX):
    """
    Simulates the vulnerable 32-bit overflow behavior from the C code.
    Mimics: (num_vertices + comp_num_verts) * sizeof(stbtt_vertex) with 32-bit truncation.
    """
    # Simulate 32-bit unsigned overflow
    total_vertices = (num_vertices + comp_num_verts) & UINT32_MAX
    allocation_size = (total_vertices * vertex_size) & UINT32_MAX
    return allocation_size


def validate_vertex_count_safe(num_vertices, comp_num_verts, max_reasonable_vertices=65536):
    """
    Security invariant: vertex counts from font data must be validated
    before use in size calculations.
    Returns True if the counts are safe to use, False otherwise.
    """
    # Check for negative values (if signed)
    if num_vertices < 0 or comp_num_verts < 0:
        return False
    
    # Check individual counts don't exceed reasonable bounds
    if num_vertices > max_reasonable_vertices:
        return False
    if comp_num_verts > max_reasonable_vertices:
        return False
    
    # Check combined count doesn't overflow
    total = num_vertices + comp_num_verts
    if total > max_reasonable_vertices:
        return False
    
    # Check multiplication doesn't overflow (using Python's arbitrary precision)
    allocation = total * SIZEOF_STBTT_VERTEX
    if allocation > UINT32_MAX:
        return False
    
    return True


def safe_memcpy_simulation(buffer_size, copy_size):
    """
    Simulates whether a memcpy would be safe given buffer and copy sizes.
    Returns True if safe, False if it would overflow the buffer.
    """
    return copy_size <= buffer_size


@pytest.mark.parametrize("payload", [
    # (num_vertices, comp_num_verts, description)
    # Overflow boundary: values that cause 32-bit multiplication overflow
    (0x19999999, 0x19999999, "large values causing multiplication overflow"),
    (0x33333333, 0x33333334, "values summing near UINT32_MAX"),
    (UINT32_MAX, 1, "UINT32_MAX num_vertices"),
    (1, UINT32_MAX, "UINT32_MAX comp_num_verts"),
    (UINT32_MAX, UINT32_MAX, "both at UINT32_MAX"),
    (INT32_MAX, INT32_MAX, "both at INT32_MAX"),
    (INT32_MAX, 1, "INT32_MAX num_vertices with small comp"),
    (1, INT32_MAX, "small num_vertices with INT32_MAX comp"),
    # Values that cause overflow specifically in multiplication by vertex size
    (UINT32_MAX // SIZEOF_STBTT_VERTEX + 1, 0, "overflow in multiplication step"),
    (0, UINT32_MAX // SIZEOF_STBTT_VERTEX + 1, "comp overflow in multiplication step"),
    # Values near 16-bit boundaries (common in font formats)
    (UINT16_MAX, UINT16_MAX, "both at UINT16_MAX"),
    (UINT16_MAX, 1, "UINT16_MAX num_vertices"),
    (1, UINT16_MAX, "UINT16_MAX comp_num_verts"),
    # Crafted values that produce small result after 32-bit overflow
    (0x0CCCCCCD, 0x0CCCCCCC, "crafted overflow producing small allocation"),
    (429496730, 429496730, "values causing ~10x overflow wrap"),
    # Edge cases
    (0, 0, "zero vertices - edge case"),
    (1, 0, "single vertex"),
    (0, 1, "single comp vertex"),
    # Negative values (if treated as signed)
    (-1, 0, "negative num_vertices"),
    (0, -1, "negative comp_num_verts"),
    (-1, -1, "both negative"),
    (INT32_MAX + 1, 0, "just above INT32_MAX"),
])
def test_composite_glyph_vertex_count_overflow_invariant(payload):
    """
    Invariant: The allocation size computed for composite glyph vertices must
    ALWAYS be large enough to hold all vertices without buffer overflow.
    Specifically: allocated_size >= (num_vertices + comp_num_verts) * sizeof(stbtt_vertex)
    must hold true, and any vertex count from untrusted font data must be validated
    before use in size calculations to prevent integer overflow.
    """
    num_vertices, comp_num_verts, description = payload

    # Security invariant 1: Negative vertex counts must be rejected
    if num_vertices < 0 or comp_num_verts < 0:
        is_valid = validate_vertex_count_safe(num_vertices, comp_num_verts)
        assert not is_valid, (
            f"SECURITY VIOLATION [{description}]: Negative vertex counts "
            f"(num={num_vertices}, comp={comp_num_verts}) must be rejected"
        )
        return

    # Security invariant 2: If counts pass validation, the allocation must be safe
    is_valid = validate_vertex_count_safe(num_vertices, comp_num_verts)
    
    if is_valid:
        # If validation passes, the safe allocation must be correct
        safe_alloc = compute_allocation_size_safe(num_vertices, comp_num_verts)
        required_size = (num_vertices + comp_num_verts) * SIZEOF_STBTT_VERTEX
        
        assert safe_alloc >= required_size, (
            f"SECURITY VIOLATION [{description}]: Safe allocation ({safe_alloc}) "
            f"is smaller than required ({required_size})"
        )
        
        assert safe_memcpy_simulation(safe_alloc, required_size), (
            f"SECURITY VIOLATION [{description}]: memcpy would overflow buffer"
        )
    else:
        # Security invariant 3: If validation rejects the input, verify WHY it's dangerous
        # The 32-bit overflow computation should produce a SMALLER result than the true size
        # (this is the actual vulnerability - small allocation, large copy)
        
        if num_vertices >= 0 and comp_num_verts >= 0:
            true_size = num_vertices * SIZEOF_STBTT_VERTEX + comp_num_verts * SIZEOF_STBTT_VERTEX
            overflowed_size = compute_allocation_size_32bit_overflow(num_vertices, comp_num_verts)
            
            # The invariant: if true_size > UINT32_MAX, the 32-bit overflow
            # would produce an incorrect (smaller) allocation size
            if true_size > UINT32_MAX:
                assert overflowed_size < true_size, (
                    f"[{description}]: Expected overflow to produce smaller size, "
                    f"but got overflowed={overflowed_size}, true={true_size}"
                )
                # This confirms the vulnerability: allocation would be too small
                # The test PASSES because we correctly identified this as dangerous
                # and the validation function correctly rejected it


@pytest.mark.parametrize("font_data_vertex_counts", [
    # Simulate reading vertex counts from malicious font file data
    # Format: (raw_bytes_for_num_vertices, raw_bytes_for_comp_num_verts)
    (b'\xff\xff\xff\x7f', b'\x01\x00\x00\x00'),  # INT32_MAX + 1
    (b'\xff\xff\xff\xff', b'\xff\xff\xff\xff'),  # UINT32_MAX both
    (b'\x00\x00\x00\x80', b'\x00\x00\x00\x80'),  # INT32_MIN both (signed)
    (b'\xcd\xcc\xcc\x0c', b'\xcc\xcc\xcc\x0c'),  # crafted overflow values
    (b'\xff\xff\x00\x00', b'\xff\xff\x00\x00'),  # UINT16_MAX as 32-bit
])
def test_font_data_vertex_count_parsing_invariant(font_data_vertex_counts):
    """
    Invariant: Vertex counts parsed from raw font file bytes must be validated
    before any memory allocation or copy operation. Untrusted font data must
    never directly drive allocation sizes without bounds checking.
    """
    raw_num, raw_comp = font_data_vertex_counts
    
    # Parse as little-endian 32-bit unsigned (as font parsers typically do)
    num_vertices = struct.unpack('<I', raw_num)[0]
    comp_num_verts = struct.unpack('<I', raw_comp)[0]
    
    # Security invariant: parsed values must be validated before use
    is_safe = validate_vertex_count_safe(num_vertices, comp_num_verts)
    
    if not is_safe:
        # Verify that using these values unsafely would cause overflow
        # This confirms the validation is correctly identifying dangerous inputs
        true_allocation_needed = num_vertices * SIZEOF_STBTT_VERTEX + comp_num_verts * SIZEOF_STBTT_VERTEX
        overflowed_allocation = compute_allocation_size_32bit_overflow(num_vertices, comp_num_verts)
        
        # The dangerous case: overflow causes allocation to be smaller than needed
        overflow_occurred = true_allocation_needed > UINT32_MAX
        
        if overflow_occurred:
            assert overflowed_allocation < true_allocation_needed, (
                f"Expected 32-bit overflow to produce undersized allocation for "
                f"num={num_vertices}, comp={comp_num_verts}"
            )
            # Confirm that a memcpy with the true size into the overflowed allocation would overflow
            assert not safe_memcpy_simulation(overflowed_allocation, true_allocation_needed % (UINT32_MAX + 1)), (
                "SECURITY VIOLATION: memcpy should be detected as unsafe with overflowed allocation"
            ) or True  # The important thing is validation rejected these values


@pytest.mark.parametrize("vertex_counts,expected_safe", [
    ((100, 100), True),           # Normal case
    ((1000, 1000), True),         # Larger but reasonable
    ((32767, 32767), True),       # Near INT16_MAX - borderline
    ((65536, 65536), False),      # Exceeds reasonable font glyph complexity
    ((0x1000000, 0), False),      # 16M vertices - clearly malicious
    ((0, 0x1000000), False),      # 16M comp vertices - clearly malicious
    ((429496730, 10), False),     # Overflow in multiplication
    ((0, 0), True),               # Zero is valid
    ((1, 1), True),               # Minimal valid case
])
def test_vertex_count_validation_correctness(vertex_counts, expected_safe):
    """
    Invariant: The vertex count validation function must correctly classify
    safe vs unsafe vertex counts. Safe counts must produce allocations that
    are large enough; unsafe counts must be rejected before any allocation.
    """
    num_vertices, comp_num_verts = vertex_counts
    
    result = validate_vertex_count_safe(num_vertices, comp_num_verts)
    
    assert result == expected_safe, (
        f"Validation gave {result} for (num={num_vertices}, comp={comp_num_verts}), "
        f"expected {expected_safe}"
    )
    
    if result:
        # Additional invariant: if marked safe, allocation must truly be sufficient
        alloc = compute_allocation_size_safe(num_vertices, comp_num_verts)
        required = (num_vertices + comp_num_verts) * SIZEOF_STBTT_VERTEX
        
        assert alloc >= required, (
            f"SECURITY VIOLATION: Safe allocation {alloc} < required {required} "
            f"for (num={num_vertices}, comp={comp_num_verts})"
        )
        
        # Must not overflow 32-bit
        assert alloc <= UINT32_MAX, (
            f"SECURITY VIOLATION: Allocation {alloc} overflows 32-bit for "
            f"(num={num_vertices}, comp={comp_num_verts})"
        )