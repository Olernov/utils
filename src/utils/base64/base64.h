#include <string>

/* ���������� ���������� ��������, ���������� � �������� ����� */
int base64_encode (unsigned char const* p_pszToEncode, unsigned int p_iLen, char *p_pszOut);
std::string base64_decode (std::string const& s);
