#include <Windows.h>
#include <wincrypt.h>
#include <vector>
#include <algorithm>

#include "Environment.h"

import Helpers;
import Crypton;

/// <summary>
/// Dekoduje ci¹g base64.
/// </summary>
/// <param name="enc_data">string = ci¹g zakodowany base64</param>
/// <returns>vector<unsigned char> = odkodowany ci¹g base64</returns>
std::vector<unsigned char>
Juche::Crypto::base64_decode(const std::string& enc_data) noexcept(false) {

	size_t len = enc_data.length();
	size_t padding = 0;

    if (enc_data[len - 1] == '=') padding++;
	if (enc_data[len - 2] == '=') padding++;


	std::vector<unsigned char> decoded;
	decoded.reserve(((enc_data.length() / 4) * 3) - padding);

	unsigned int temp = 0;
	auto it = enc_data.begin();

	while (it < enc_data.end())
	{
		for (std::size_t i = 0; i < 4; ++i)
		{
			temp <<= 6;
			if (*it >= 0x41 && *it <= 0x5A) {
				temp |= *it - 0x41;
			} else if (*it >= 0x61 && *it <= 0x7A) {
				temp |= *it - 0x47;
			} else if (*it >= 0x30 && *it <= 0x39) {
				temp |= *it + 0x04;
			} else if (*it == 0x2B) {
				temp |= 0x3E;
			} else if (*it == 0x2F) {
				temp |= 0x3F;
			} else if (*it == '=') {
				
				switch (enc_data.end() - it)
				{
				        case 1:
					          decoded.push_back((temp >> 16) & 0x000000FF);
					          decoded.push_back((temp >> 8) & 0x000000FF);
					          return decoded;
				        case 2:
					          decoded.push_back((temp >> 10) & 0x000000FF);
					          return decoded;
				}
			}

			++it;
		}

		decoded.push_back((temp >> 16) & 0x000000FF);
		decoded.push_back((temp >> 8) & 0x000000FF);
		decoded.push_back((temp) & 0x000000FF);

	}

	return decoded;

}

/// <summary>
/// Koduje przekazane dane przy mocony base64.
/// </summary>
/// <param name="data">unsigned char* = dane do zakodowania.</param>
/// <param name="data_size">DWORD = rozmiar danych do zakodowania</param>
/// <returns>string = ci¹g base64.</returns>
std::string 
Juche::Crypto::base64_encode(const BYTE* data, DWORD data_size) noexcept(false) {
	  
	std::string ret = "";
	
	const BYTE* input = data;
	const unsigned char* end = data + data_size;

	size_t len = data_size * 4 / 3 + 4;
	len += len + 1;

	if (len <= data_size) {
		return ret;
	}

	char* buff = new char[len];
	if (buff != nullptr) {

		//ZeroMemory(buff, len);

		char* pos = buff;

		while (end - input >= 3) {

			*pos++ = Juche::Crypto::BASE64_table[input[0] >> 2];
			*pos++ = Juche::Crypto::BASE64_table[((input[0] & 0x03) << 4) | (input[1] >> 4)];
			*pos++ = Juche::Crypto::BASE64_table[((input[1] & 0x0f) << 2) | (input[2] >> 6)];
			*pos++ = Juche::Crypto::BASE64_table[input[2] & 0x3f];

			input += 3;
		}
		
		if (end - input) {
			
			*pos++ = Juche::Crypto::BASE64_table[input[0] >> 2];
			if (end - input == 1) {
				*pos++ = Juche::Crypto::BASE64_table[(input[0] & 0x03) << 4];
				*pos++ = '=';
			} else {
				*pos++ = Juche::Crypto::BASE64_table[((input[0] & 0x03) << 4) | (input[1] >> 4)];
				*pos++ = Juche::Crypto::BASE64_table[(input[1] & 0x0f) << 2];
			}
			*pos++ = '=';
			
		}

		*pos++ = '\0';
		ret = buff;

		delete[] buff;

	}

	return ret;
}

/// <summary>
/// Oblicza sumê kontroln¹ CRC32 dla przekazanych 
/// danych.
/// </summary>
/// <param name="init_val">Wartoœæ pocz¹tkowa</param>
/// <param name="data">WskaŸnik do danych.</param>
/// <param name="len">Wielkoœæ danych.</param>
/// <returns>Suma kontrolna CRC32.</returns>
DWORD Juche::Crypto::crc32(DWORD init_val, const BYTE* data, size_t len) noexcept {

	DWORD crc = ~init_val;
    for(size_t i=0; i<len; i++) {
		crc = Juche::Crypto::CRC_table[(crc ^ data[i]) & 0xff] ^ (crc >> 8);
	}
	return ~crc;

}

BOOL 
Juche::Crypto::Cipher::private_key(DWORD size) noexcept {

	if(!CryptGenKey(provider_, CALG_DH_EPHEM, size << 16, &priv_key_)) {
		return FALSE;
	}

	CRYPT_INTEGER_BLOB _VAL;
	_VAL.cbData = sizeof(Juche::Crypto::P_table);
	_VAL.pbData = const_cast<BYTE*>(reinterpret_cast<const BYTE*>(&Juche::Crypto::P_table));

	if(!CryptSetKeyParam(priv_key_, KP_P, reinterpret_cast<const BYTE*>(&_VAL), 0)) {
		return FALSE;
    }

	_VAL.cbData = sizeof(Juche::Crypto::G_table);
	_VAL.pbData = const_cast<BYTE*>(reinterpret_cast<const BYTE*>(&Juche::Crypto::G_table));

	if (!CryptSetKeyParam(priv_key_, KP_G, reinterpret_cast<const BYTE*>(&_VAL), 0)) {
		return FALSE;
	}

	return CryptSetKeyParam(priv_key_, KP_X, NULL, 0);

}

/// <summary>
/// Konstruktor.
/// </summary>
Juche::Crypto::Cipher::Cipher(SymmetricAlgorithm enc_algo, DWORD flag) noexcept : session_key_(0), init_(false) {

	CryptAcquireContext(&provider_, NULL,
		                MS_ENH_DSS_DH_PROV,
		                PROV_DSS_DH,
		                CRYPT_VERIFYCONTEXT);
	if (private_key(flag) != FALSE) {
		enc_algo_ = enc_algo;
		init_ = true;
	}

}

/// <summary>
/// Destruktor klasy Cipher.
/// </summary>
Juche::Crypto::Cipher::~Cipher() {
  
	if (priv_key_ != 0) {
		CryptDestroyKey(priv_key_);
	}

	if (session_key_ != 0) {
		CryptDestroyKey(session_key_);
	}

	if (provider_ != 0) {
		CryptReleaseContext(provider_, 0);
    }

}

/// <summary>
/// Eksportuje klucz do postaci binarnej.
/// </summary>
/// <param name="key_type">Typ klucza.</param>
/// <param name="key_encryption_key">Klucz szyfrowania klucza.</param>
/// <returns>Klucz: jesli key.size == 0 to error, jeœli nie to ok.</returns>
Juche::Crypto::exported_key 
Juche::Crypto::Cipher::key_blob(Juche::Crypto::KeyType key_type, HCRYPTKEY key_encryption_key) const noexcept(false) {
	
	exported_key key = { 0, nullptr };

    DWORD size;
    DWORD blob_type = (key_type == Juche::Crypto::KeyType::PRIVATE) ? SIMPLEBLOB : PUBLICKEYBLOB;
	
	if (CryptExportKey(priv_key_, key_encryption_key, blob_type, CRYPT_BLOB_VER3, NULL, &size)) {

		BYTE* buff = new BYTE[size];
		if (buff != nullptr) {
			if (CryptExportKey(priv_key_, key_encryption_key, blob_type, CRYPT_BLOB_VER3, buff, &size)) {
				key.size = size;
				key.blob = buff;
			}
		}

	}

	return key;

}

/// <summary>
/// Dekoduje ci¹g base64 i zwraca w posiaci binary_string.
/// </summary>
/// <param name="encoded_key">Base64</param>
/// <returns>zdekodowany base64</returns>
binary_string 
Juche::Crypto::Cipher::decode(const std::string& encoded_key) const noexcept(false) {
	   
	std::vector<unsigned char> dec_v = Juche::Crypto::base64_decode(encoded_key);
	return binary_string(dec_v.begin(), dec_v.end());

}

int 
Juche::Crypto::Cipher::import_3rd_public_key(const BYTE* key_blob, DWORD blob_size) noexcept {

	if (!CryptImportKey(provider_, key_blob, blob_size, priv_key_, 0, const_cast<HCRYPTKEY*>(&session_key_))) {
		DWORD err = GetLastError();
		if (err == ERROR_INVALID_HANDLE || err == ERROR_INVALID_PARAMETER || err == ERROR_BUSY) {
			return -1;
		}
		return 0;
	}
	return (algorithm(enc_algo_)) ? 1 : 0;

}

/// <summary>
/// Rozmiar bloku dla szyfrów blokowych.
/// </summary>
/// <param name="key"></param>
/// <returns></returns>
DWORD 
Juche::Crypto::Cipher::block_size(HCRYPTKEY key) const noexcept {

	DWORD block_size, block_size_len = sizeof(DWORD);
	if (!CryptGetKeyParam(key, KP_BLOCKLEN, (BYTE*)&block_size, &block_size_len, 0)) {
		return 1;
	}
	return block_size;

}

/// <summary>
/// Szyfruje dane przy pomocy wybranego algorytmu.
/// Nie obs³uguje szyfrowania danych podzielonych na bloki, w 
/// takim przypdaku trzeba podzieliæ dane na bloki, wywo³aæ Cipher::encrypt osobno dla ka¿dgo bloku i 
/// po³¹czyæ wyniki samodzielne.
/// Obs³uga bloków z wartoœci¹ final na FALSE jak przedstawiono mn. tu:
/// https://docs.microsoft.com/en-us/windows/win32/seccrypto/example-c-program-encrypting-a-file
/// Nie jest zwi¹zana z szyframi blokowymi, ale z sytuacj¹ gdy dane s¹ np. zbyt du¿e aby 
/// umieœciæ je w pamiêci za jednym razem.
/// </summary>
/// <param name="data">WskaŸnik do danych, które chcemy zaszyfrowaæ.</param>
/// <param name="data_len">Rozmiar danych do zaszyfrowania.</param>
/// <returns>
/// Szyfrogram w postaci wektora znaków lub pusty wektor w przypadku 
/// niepowodzenia. 
/// </returns>
std::vector<unsigned char> 
Juche::Crypto::Cipher::encrypt(const BYTE* data, DWORD data_len) const noexcept(false) {  

	DWORD len = data_len;

	if (CryptEncrypt(session_key_, 0, TRUE, 0, NULL, &len, data_len) == FALSE) {
		return std::vector<unsigned char>(0);
	};

	BYTE* buff = new BYTE[len];
	if (buff == nullptr) {
		return std::vector<unsigned char>(0);
	}

	ZeroMemory(buff, len);

	memcpy(&buff[0], &data[0], len);

	if (!CryptEncrypt(session_key_, 0, TRUE, 0, buff, &data_len, len)) {
		return std::vector<unsigned char>(0);
	}

	std::vector<unsigned char> ret(0);
	std::copy(buff, buff + data_len, std::back_inserter(ret));
	
	delete[] buff;

	return ret;
}

bool 
Juche::Crypto::Cipher::encryption_mode(DWORD mode) noexcept {
	  
	return CryptSetKeyParam(
		session_key_,
		KP_MODE, reinterpret_cast<const BYTE*>(&mode), 0
	);

}

/// <summary>
/// Deszyfruje dane przy pomocy wybranego algorytmu.
/// Nie obs³uguje deszyfrowania danych podzielonych na bloki, w 
/// takim przypdaku trzeba podzieliæ dane na bloki, wywo³aæ Cipher::encrypt osobno dla ka¿dgo bloku i 
/// po³¹czyæ wyniki samodzielne.
/// Obs³uga bloków z wartoœci¹ final na FALSE jak przedstawiono mn. tu:
/// https://docs.microsoft.com/en-us/windows/win32/seccrypto/example-c-program-decrypting-a-file
/// Nie jest zwi¹zana z szyframi blokowymi, ale z sytuacj¹ gdy dane s¹ np. zbyt du¿e aby 
/// umieœciæ je w pamiêci za jednym razem.
/// </summary>
/// <param name="enc_data">WskaŸnik do zaszyfrowanych danych.</param>
/// <param name="enc_data_len">Wielkoœæ zaszyfrowanych danych.</param>
/// <returns>
/// Odszyfrowane dane w postaci wektora znaków lub pusty wektor w przypadku 
/// niepowodzenia. 
/// </returns>
std::vector<unsigned char> 
Juche::Crypto::Cipher::decrypt(const BYTE* enc_data, DWORD enc_data_len) const noexcept(false) {   
	
	DWORD len = enc_data_len;

	BYTE* buff = new BYTE[len];
	if (buff == nullptr) {
		return std::vector<unsigned char>(0);
	}

	ZeroMemory(buff, len);
	memcpy(&buff[0], &enc_data[0], enc_data_len);

	std::vector<unsigned char> ret(0);

	if (CryptDecrypt(session_key_, 0, TRUE, 0, buff, &len)) {
		std::copy(buff, buff + len, std::back_inserter(ret));
	}

	delete[] buff;

	return ret;

}

std::vector<unsigned char> 
Juche::Crypto::Cipher::decrypt(Juche::Crypto::SymmetricAlgorithm enc_algo, const std::string& key, const BYTE* enc_data, DWORD enc_data_len) const noexcept(false) {

	HCRYPTHASH hash;
	std::vector<unsigned char> ret(0);
	
	if (CryptCreateHash(provider_, CALG_SHA1, 0, 0, &hash) == FALSE) {
		return ret;
	}

	if (CryptHashData(hash, reinterpret_cast<BYTE*>(const_cast<CHAR*>(key.c_str())), static_cast<DWORD>(key.length()), 0) == TRUE) {

		HCRYPTKEY key;
		if (CryptDeriveKey(provider_, static_cast<ALG_ID>(enc_algo), hash, 0, &key) == TRUE) {

			DWORD len = enc_data_len;

			BYTE* buff = new BYTE[len];
			if (buff != nullptr) {
				
				ZeroMemory(buff, len);
				memcpy(&buff[0], &enc_data[0], enc_data_len);

				if (CryptDecrypt(key, 0, TRUE, 0, buff, &len)) {
					std::copy(buff, buff + len, std::back_inserter(ret));
				}

				delete[] buff;
			}

			CryptDestroyKey(key);

		}

	}
	
	CryptDestroyHash(hash);

	return ret;

}

std::vector<unsigned char> 
Juche::Crypto::Cipher::encrypt(Juche::Crypto::SymmetricAlgorithm enc_algo, 
	                           const std::string& key, const BYTE* data, DWORD data_len) const noexcept(false) {

	HCRYPTHASH hash;
	std::vector<unsigned char> ret(0);

	if (CryptCreateHash(provider_, CALG_SHA1, 0, 0, &hash) == FALSE) {
		return ret;
	}
	
	if (CryptHashData(hash, reinterpret_cast<BYTE*>(const_cast<CHAR*>(key.c_str())), static_cast<DWORD>(key.length()), 0) == TRUE) {

		HCRYPTKEY key;
		if (CryptDeriveKey(provider_, static_cast<ALG_ID>(enc_algo), hash, 0, &key) == TRUE) {

			DWORD len = data_len;
			if (CryptEncrypt(key, 0, TRUE, 0, NULL, &len, data_len) == TRUE) {
				BYTE* buff = new BYTE[len];
				if (buff != nullptr) {

					memcpy(&buff[0], &data[0], len);

					if (CryptEncrypt(key, 0, TRUE, 0, buff, &data_len, len) == TRUE) {
						std::copy(buff, buff + data_len, std::back_inserter(ret));
					}

					delete[] buff;
				}
			}

			CryptDestroyKey(key);
		}

	}

	CryptDestroyHash(hash);
	return ret;

}