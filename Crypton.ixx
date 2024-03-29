///////////////////////////////////////
/// 26/06/2022 21:57
/// Modu� udost�pnia klas� kt�ra umo�liwia symetryczne 
/// szyfrowanie i deszyfrowanie danych z bezpieczn� wymian� klucza
/// bazuj�c� na protokole Diffiego-Hellmana.
/// Klasa umo�liwia tylko eksport klucza publicznego
/// (w formacie binarnym oraz binarnym zakodwoanym base64).
/// W zwi�zku z czym wykorzystywane klucze s� aktualne tylko per sesja/instacja obiektu.
/// Wspiera 6 symetrycznych algorytm�w szyfrowania (tyle udost�pnia wykorzstywany CSP), 
/// umo�liwia r�wnie� zmian� trybu szyfrowania.
/// Aby zaszyfrowa� dane, poza wymian� klucza publicznego, trzeba oczywi�cie 
/// przekaza� informacje o tym jakim algorytmem zosta�y zaszyforwane.
/// Klasa umo�liwia r�wnie� szyfrowanie i deszyfrowanie wybranym algorytmem z kluczem 
/// tajnym, kt�ry mo�e by� sta�y, lub mie� d�u�szy czas �ycia ni� klucze DH.
/// Modu� zawiera r�wnie� funkcje CRC32 oraz base64.
////////////////////////////////////////

module;

#include <Windows.h>
#include <vector>

#include "Environment.h"

export module Crypton;

namespace Juche {

	namespace Crypto {

		export {

            /// Base64
            /// //////////
            /// 

            extern const std::string BASE64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

            /// /////////

            std::string base64_encode(const BYTE* data, DWORD data_size) noexcept(false);

            std::vector<unsigned char> base64_decode(const std::string& enc_data) noexcept(false);

            /// CRC32
            /// //////////
            /// 

            extern const DWORD CRC_table[] =
            {
                0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
                0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
                0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
                0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
                0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
                0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
                0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
                0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
                0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
                0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
                0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
                0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
                0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
                0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
                0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
                0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
                0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
                0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
                0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
                0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
                0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
                0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
                0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
                0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
                0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
                0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
                0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
                0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
                0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
                0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
                0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
                0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
                0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
                0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
                0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
                0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
                0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
                0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
                0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
                0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
                0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
                0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
                0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
            };

			DWORD crc32(DWORD init_val, const BYTE* data, size_t len) noexcept;

            /// <summary>
            /// Liczby pierwsze.
            /// </summary>
            
            extern const BYTE P_table[] = {
                   0x39, 0x6b, 0xc7, 0x7c, 0xe9, 0x20, 0xd9, 0x51, 0xba, 0x81, 0x04, 0x95,
                   0xf2, 0x23, 0x38, 0x08, 0xc5, 0x01, 0x8c, 0x7d, 0xbf, 0x11, 0xe5, 0xbc,
                   0x22, 0xb0, 0x41, 0x9c, 0x00, 0x6c, 0xd9, 0x8c, 0x32, 0x2b, 0xc6, 0x5b,
                   0x8b, 0xc5, 0xbb, 0x7d, 0x29, 0x92, 0x01, 0x17, 0x58, 0xde, 0x91, 0x45,
                   0xb2, 0x08, 0x06, 0x5d, 0xcc, 0x4d, 0x4e, 0x9e, 0x49, 0x86, 0x7f, 0x85,
                   0xb5, 0xf1, 0x11, 0xc0
            };

            extern const BYTE G_table[] = {
                   0x1a, 0x4d, 0x90, 0xe0, 0x30, 0x32, 0x5e, 0xdc, 0x5e, 0x91, 0x60,
                   0x76, 0x62, 0x2a, 0xb4, 0xc1, 0xce, 0x38, 0x93, 0x8c, 0xa7, 0x91,
                   0x5d, 0x52, 0x7f, 0xb8, 0x8d, 0x55, 0x86, 0xbc, 0xa9, 0x00, 0x1d,
                   0x36, 0x63, 0x3e, 0xbc, 0xf6, 0x53, 0xd9, 0x33, 0xe6, 0x6f, 0x60,
                   0xc6, 0x84, 0x71, 0x67, 0xef, 0x86, 0x67, 0x60, 0x5e, 0xe9, 0xcf,
                   0x6f, 0xfd, 0x85, 0xac, 0x9f, 0x99, 0x2d, 0xea, 0x42
            };

            /// <summary>
            /// blob jest alokowany przy pomocy operatora new[]
            /// i powinien/musi� by� zwolniony za pomoc� delete[].
            /// blob = klucz w formie binarnej 
            /// size = rozmiar klucza
            /// </summary>
            struct exported_key {
                DWORD size;
                BYTE* blob;
            };

            void release_exported_key(const exported_key& key) noexcept {
                ZeroMemory(key.blob, key.size);
                delete[] key.blob;
            }

            ///
            /// /////////
            /// 
           
            /// <summary>
            /// Typy kluczy jakie nas interesuj�.
            /// </summary>
            enum class KeyType : unsigned char {
                PUBLIC,
                PRIVATE
            };

            /// <summary>
            /// Obs�ugiwane symetryczne algorytmy 
            /// szyfrowania.
            /// </summary>
            enum class SymmetricAlgorithm : ALG_ID {
                ALG_CYLINK_MEK = CALG_CYLINK_MEK,
                ALG_RC4 = CALG_RC4,
                ALG_RC2 = CALG_RC2,
                ALG_3DES = CALG_3DES,
                ALG_3DES_112 = CALG_3DES_112,
                ALG_DES = CALG_DES
            };
           
            /// 
            /// //////////
            /// 
            
            /// <summary>
            /// Ubogi Defi Helmans - Umo�liwia "bezpieczn�" wymian� klucza i 
            /// szyfrowanie symetryczne przy pomocy jednego z kilku algorytm�w.
            /// Klasa nie zapewnia mo�lio�ci eksportu/importu klucza prywatnego
            /// wi�c generowana para kluczy pub/priv oraz klucz sesyjny jest w�a�ciwie efemeryczna.
            /// Oczywi�cie nic nie stoi na przeszkodzie aby zaszyfrowa� nimi inny klucz symetryczny (has�o)
            /// i si� nim wymieni� z drug� stron�.
            /// Klasa posiada dwie przeci��one metody encrypt i decrypt, kt�re umo�liwiaj� 
            /// szyfrowanie tajnym kluczem, kt�ry mo�e by� sta�y.
            /// </summary>
            class Cipher {

                  bool init_;

                  SymmetricAlgorithm enc_algo_;
                  
                  HCRYPTPROV provider_;
                  
                  /// <summary>
                  /// Nasze klucze prywatny i publiczny.
                  /// </summary>
                  HCRYPTKEY priv_key_;
                 
                  /// <summary>
                  /// Klucz szyfrowania danych.
                  /// </summary>
                  HCRYPTKEY session_key_;

                  /// <summary>
                  /// Helpers.
                  /// </summary>
                  
                  /// <summary>
                  /// Zwraca rozmiar bloku dla szyfr�w blokowych.
                  /// Dla szyfr�w strumieniowych zwraca 1
                  /// (w orginale zwraca 0, ale mno�y� przez 0 to do�� g�upie).
                  /// </summary>
                  /// <param name="key">Klucz (sessji).</param>
                  /// <returns>Rozmiar bloku lub 1</returns>
                  DWORD block_size(HCRYPTKEY key) const noexcept;

                  BOOL private_key(DWORD size) noexcept;

                  inline bool algorithm(SymmetricAlgorithm algo) noexcept {
                      return CryptSetKeyParam(session_key_, KP_ALGID, reinterpret_cast<const BYTE*>(&algo), 0);
                  }

            public:
                  
                  explicit Cipher(SymmetricAlgorithm enc_algo, DWORD size = 512) noexcept;

                  exported_key key_blob(KeyType key_type = KeyType::PUBLIC, HCRYPTKEY key_encryption_key = 0) const noexcept(false);

                  inline std::string encode(const exported_key& key) const noexcept(false) {
                      return Juche::Crypto::base64_encode(key.blob, key.size);
                  }

                  binary_string decode(const std::string& encoded_key) const noexcept(false);

                  int import_3rd_public_key(const BYTE* key_blob, DWORD blob_size) noexcept;

                  /// <summary>
                  /// Ustawia tryb szyfrowania.
                  /// Domy�lny tryb CRYPT_MODE_CBC.
                  /// Dost�pne tryby w zale�n�ci od stosowanego algorytmu:
                  /// CRYPT_MODE_CBC
                  /// CRYPT_MODE_CFB
                  /// CRYPT_MODE_ECB
                  /// CRYPT_MODE_OFB
                  /// CRYPT_MODE_CTS
                  /// </summary>
                  bool encryption_mode(DWORD mode) noexcept;

                  /// <summary>
                  /// Szyfrowanie kluczem efemerycznym.
                  /// </summary>
                  std::vector<unsigned char> encrypt(const BYTE* data, DWORD data_len) const noexcept(false);

                  /// <summary>
                  /// Szyfruje dane wskazanym algorytmem z u�yciem 
                  /// klucza przekazanego w argumencie.
                  /// </summary>
                  /// <param name="enc_algo">Algorytm szyfrowania.</param>
                  /// <param name="key">Klucz szyfrowania.</param>
                  /// <param name="data">Dane do zaszyfrowania</param>
                  /// <param name="data_len">Rozmiar danych do zaszyfrowania.</param>
                  /// <returns>
                  /// Vector binarnych znak�w szyfrogramu.
                  /// Je�li rozmiar vector == 0 to znaczy, �e co� posz�o �le,
                  /// wi�cej informacji zwr�ci GetLastError().
                  /// </returns>
                  std::vector<unsigned char> encrypt(SymmetricAlgorithm enc_algo, const std::string& key, const BYTE* data, DWORD data_len) const noexcept(false);

                  /// <summary>
                  /// Deszyfrowanie kluczem efemerycznym.
                  /// </summary>
                  std::vector<unsigned char> decrypt(const BYTE* enc_data, DWORD enc_data_len) const noexcept(false);

                  /// <summary>
                  /// Deszyfruje dane wskazanym algorytmem z u�yciem 
                  /// klucza przekazanego w argumencie.
                  /// </summary>
                  /// <param name="enc_algo">Algorytm szyfrowania.</param>
                  /// <param name="key">Klucz szyfrowania.</param>
                  /// <param name="enc_data">Zaszyfrowane dane.</param>
                  /// <param name="enc_data_len">Rozmiar zaszyfrowanych danyc.</param>
                  /// <returns>
                  /// Vector binarnych znak�w odszyfrowanej wiadomo�ci.
                  /// Je�li rozmiar vector == 0 to znaczy, �e co� posz�o �le,
                  /// wi�cej informacji zwr�ci GetLastError().
                  /// </returns>
                  std::vector<unsigned char> decrypt(SymmetricAlgorithm enc_algo, const std::string& key, const BYTE* enc_data, DWORD enc_data_len) const noexcept(false);

                  inline bool init() const noexcept {
                      return init_;
                  }

                  ~Cipher();

            };

		}

    }

}