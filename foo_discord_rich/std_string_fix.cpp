// Dummy implementations for missing std library symbols in VS2026
// These are internal optimization functions that should be inlined
// but are referenced as external symbols in cpr.lib

extern "C"
{
    // Dummy implementation for __std_find_first_not_of_trivial_pos_1
    unsigned __int64 __std_find_first_not_of_trivial_pos_1(
        const void* _First1,
        const void* _Last1,
        const void* _First2,
        const void* _Last2 )
    {
        // Return not found
        return static_cast<unsigned __int64>( -1 );
    }

    // Dummy implementation for __std_find_last_not_of_trivial_pos_1
    unsigned __int64 __std_find_last_not_of_trivial_pos_1(
        const void* _First1,
        const void* _Last1,
        const void* _First2,
        const void* _Last2 )
    {
        // Return not found
        return static_cast<unsigned __int64>( -1 );
    }
}
