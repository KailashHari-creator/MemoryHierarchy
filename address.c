uint32_t get_page_offset(uint32_t va)
{
    return va & 0x3FFU;
}
uint32_t get_vpn(uint32_t va)
{
    return va >> 10;
}
uint32_t make_physical_address(uint32_t pfn, uint32_t offset)
{
    return (pfn << 10) | offset;
}