////////////////////////////////////////////////////////////////////////////
///21/04/2022 17:12
///Modu� umo�liwia pobranie regu� zapory sieciowej oraz sprawdzenie 
///na podstawie �cie�ki do pliku czy dana aplikacja ma mo�liwo�� odbierania 
///lub nawi�zywania po��cze� z okre�lonych i do okre�lonych adres�w IP na okre�lonych portach sieciowych.
///Klasy maj� mo�liwo�� relatywnie �atwego rozszerzenia o mo�liwo�ci edycji i definiowania 
///nowych regu� zapory sieciowej.
////////////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include <vector>
#include <netfw.h>
#include <string>
#include <atlcomcli.h>
#include <stdexcept>
#include <regex>
#include <memory>

#include "Environment.h"

import Firewall;
import Helpers;

/// <summary>
/// Destruktor klasy Firewall
/// </summary>
Juche::Firewall::Firewall::~Firewall() {

	raw_release<Firewall>(this);

	if (fw.use_count() == 1) {
	
	    if ((*fw) != nullptr) {
		    (*fw)->Release();
	    }
    
    }

	if (aa != nullptr) {
		delete aa;
	}

	if (com_uninit == true) {
		CoUninitialize();
	}

}

/// <summary>
/// Metoda weryfikuje czy regu�a firewall-a spe�nia okre�lone warunki:
/// Czy adresy ip z kt�rymi mo�na si� po��czy� s� zgodne z oczekiwaniami zdefiniowanymi jako wyra�enie reguralne.
/// Czy porty lokalne i/lub zdalne s� zgodne z oczekiwaniami zdefiniowanymi jako wyra�enie reguralne.
/// Czy protok� komunikacyjny jest zgodny z oczekiwanym.
/// Czy kierunek po��czenia jest zgodny z oczekiwanym. 
/// Czy regu�a dopuszcza b�d� blokuje okre�lony ruch.
/// Czy profil w kt�rym zdefiniowana jest regu�a zapory jest zgodny z oczekiwanym.
/// </summary>
/// <param name="rule">
/// INetFwRule to pojedy�cza regu�a zdefiniowana w zaporze sieciowej MS Windows.
/// Warto�� uzyskana przez iterator struktury INetFwRules.
/// </param>
/// <param name="raddr_pattern">
/// Wra�enie reguralne, kt�re weryfikuje czy warto�� uzyskana przez wywo�anie get_RemoteAddresses 
/// jest zgodna z oczekiwaniami. 
/// </param>
/// <param name="rport_pattern"></param>
/// Wra�enie reguralne, kt�re weryfikuje czy warto�� uzyskana przez wywo�anie get_RemotePorts
/// jest zgodna z oczekiwaniami
/// <param name="lport_pattern">
/// Wra�enie reguralne, kt�re weryfikuje czy warto�� uzyskana przez wywo�anie get_LocalPorts
/// jest zgodna z oczekiwaniami.
/// </param>
/// <param name="protocol">
/// Warto�� do por�wnania z wynikiem wywo�ania get_Protocol.
/// </param>
/// <param name="direction">
/// Kierunek po��czenia, ta warto�� jest por�wnywana z wynikiem 
/// wywo�ania get_Direction.
/// </param>
/// <param name="action">
/// Warto�� por�wnywana z wynikiem wywo�ania get_Action.
/// </param>
/// <param name="profile">
/// Warto�� por�wnywana z wynikiem wywo�ania get_Profiles.
/// </param>
/// <returns>
/// -1 = error (b��d wywo�ania metod/funckji), mo�na rzuci� wyj�tkiem.
///  0 = false (regu�a nie spe�nia oczekiwa�).
///  1 = true  (regu�a zgodna z oczekiwaniami, 
///  informacje zostan� umieszczone w sk�adowej std::vector<Settings<INetFwRule>> policies).
/// </returns>
int 
Juche::Firewall::Firewall::parse(INetFwRule* rule, const string_t& raddr_pattern, const string_t& rport_pattern,
	                             const string_t& lport_pattern, NET_FW_IP_PROTOCOL protocol, TrafficType direction, 
	                             NET_FW_ACTION action, NET_FW_PROFILE_TYPE2 profile) noexcept(false) {

#ifdef __J_DEBUG__
	Juche::Helpers::DebugConsole* dbg = Juche::Helpers::DebugConsole::instance();
#endif

	VARIANT_BOOL enabled;
	if (!SUCCEEDED(rule->get_Enabled(&enabled))) {
		return -1;
	}

	if (enabled == VARIANT_FALSE) {
		return 0;
	}

	long fw_profile;
	if (!SUCCEEDED(rule->get_Profiles(&fw_profile))) {
		return -1;
	}

	if (!(fw_profile & profile)) {
		return 0;
	}

	NET_FW_RULE_DIRECTION traffic;
	if (!SUCCEEDED(rule->get_Direction(&traffic))) {
		return -1;
	}

	if (direction != TrafficType::ANY) {

		if ((traffic == NET_FW_RULE_DIR_OUT && direction != TrafficType::OUTBOUND) ||
			(traffic == NET_FW_RULE_DIR_IN && direction != TrafficType::INBOUND)) {
			return 0;
		}

	}

	LONG ip_protocol;
	if (!SUCCEEDED(rule->get_Protocol(&ip_protocol))) {
		return -1;
	}

	if (protocol != NET_FW_IP_PROTOCOL_ANY) {

		if (ip_protocol != NET_FW_IP_PROTOCOL_TCP && protocol == NET_FW_IP_PROTOCOL_TCP) {
			return 0;
		}

		if (ip_protocol != NET_FW_IP_PROTOCOL_UDP && protocol == NET_FW_IP_PROTOCOL_UDP) {
			return 0;
		}

	}

	BSTR remote_address;
	if (!SUCCEEDED(rule->get_RemoteAddresses(&remote_address))) {
		return -1;
	}

	BSTR remote_ports;
	if (!SUCCEEDED(rule->get_RemotePorts(&remote_ports))) {
		return -1;
	}

	BSTR local_ports;
	if (!SUCCEEDED(rule->get_LocalPorts(&local_ports))) {
		return -1;
	}

#ifdef UNICODE   

	std::wstring n_address(remote_address);

#else

	std::unique_ptr<char[]> tmp_address(Juche::Helpers::U2A(remote_address));
	std::string n_address = tmp_address.get();

#endif

    regex_t regexp(raddr_pattern);
    if (!std::regex_match(n_address, regexp)) {
	    return 0;
    }

    if (remote_ports != nullptr) {

#ifdef UNICODE

	    std::wstring n_rports(remote_ports);

#else

	    std::unique_ptr<char[]> tmp_rports(Juche::Helpers::U2A(remote_ports));
	    std::string n_rports = tmp_rports.get();

#endif
	    regex_t regexp(rport_pattern);
	    if (!std::regex_match(n_rports, regexp)) {
	     	return 0;
	    }

    }

    if (local_ports != nullptr) {

#ifdef UNICODE   

	   std::wstring n_lports(local_ports);

#else

	   std::unique_ptr<char[]> tmp_lports(Juche::Helpers::U2A(local_ports));
	   std::string n_lports = tmp_lports.get();

#endif
	   regexp.assign(lport_pattern);
	   if (!std::regex_match(n_lports, regexp)) {
		   return 0;
	   }

    }

    NET_FW_ACTION rule_action;
    if (!SUCCEEDED(rule->get_Action(&rule_action))) {
	    return -1;
    }

    if (rule_action != action) {
	    return 0;
    }

    BSTR app_name = nullptr;
    if (!SUCCEEDED(rule->get_ApplicationName(&app_name))) {
	    return -1;
    }

#ifndef UNICODE   

    std::string  n_appname("");
#else

    std::wstring n_appname(L"");

#endif

    if (app_name != nullptr) {

#ifndef UNICODE   

	    std::unique_ptr<CHAR[]> tmp_appname(Juche::Helpers::U2A(app_name));
	    n_appname = tmp_appname.get();

#else

	    n_appname = app_name;

#endif

    }

    BSTR rule_name;
    if (!SUCCEEDED(rule->get_Name(&rule_name))) {
	    return -1;
    }

#ifndef UNICODE 

	std::unique_ptr<CHAR[]> tmp_rulename(Juche::Helpers::U2A(rule_name));
	std::string n_rulename = tmp_rulename.get();

#else

	std::wstring n_rulename = rule_name;

#endif

	Settings<INetFwRule> setting;

	setting.direction = direction;
	setting.raw = rule;
	setting.app_path = n_appname;
	setting.name = n_rulename;
	setting.protocol = static_cast<NET_FW_IP_PROTOCOL>(ip_protocol);

#ifdef __J_DEBUG__
	
	dbg->write(
		       TEXT(
				    "Juche::Firewall::Firewall::parse() Settings:\r\n"
			        "setting.direction: 0x%x\r\n"
				    "setting.raw: 0x%p\r\n"
				    "setting.app_path: %s\r\n"
				    "setting.protocol: 0x%x\r\n"
				    "setting.name: %s\r\n"
				    "==========================\r\n"
			   ), setting.direction, setting.raw, setting.app_path.c_str(), setting.protocol, setting.name.c_str()
	);

#endif

	policies.push_back(setting);

	return 1;

}

/// <summary>
/// Tworzy wska�nik do INetFwRules, nast�pnie tworzy iterator i 
/// przechodzi przez kolejne regu�y zapory sieciowej, tworz�c wska�nik do INetFwRule kt�ry reprezentuje 
/// ka�d�, kolejn� regu�� firewall-a i jest przekazywany do metody int Juche::Firewall::Firewall::parse.
/// </summary>
/// <param name="raddr_pattern">
/// Wra�enie reguralne, kt�re weryfikuje czy warto�� uzyskana przez wywo�anie get_RemoteAddresses 
/// jest zgodna z oczekiwaniami. 
/// Parametr przekazywany do metody parse.
/// </param>
/// <param name="rport_pattern">
/// Wra�enie reguralne kt�re weryfikuje wzorzec zdalnych port�w sieciowych powi�zanych z regu��, parametr przekazywany
/// do metody parse.
/// </param>
/// <param name="lport_pattern">
/// Wra�enie reguralne kt�re weryfikuje wzorzec lokalnych port�w sieciowych powi�zanych z regu��, 
/// parametr przekazywany do metody parse.
/// </param>
/// <param name="protocol">
/// Parametr przekazywany do metody parse.
/// </param>
/// <param name="direction">
/// Parametr przekazywany do metody parse.
/// </param>
/// <param name="action">
/// Parametr przekazywany do metody parse.
/// </param>
/// <param name="profile">
/// Parametr przekazywany do metody parse.
/// </param>
/// <returns>
/// Warto�� HRESULT zwr�cona przez ostatnie nieporawne wywo�anie metody COM lub
/// S_OK (w przypadku kiedy podczas wywo�ywania metody nie pojawi� si� �aden b��d).
/// </returns>
HRESULT 
Juche::Firewall::Firewall::rules(const string_t& raddr_pattern, 
	                             const string_t& rport_pattern,
	                             const string_t& lport_pattern,
	                             NET_FW_IP_PROTOCOL protocol, TrafficType direction,
	                             NET_FW_ACTION action, NET_FW_PROFILE_TYPE2 profile) noexcept(false) {

	INetFwRules* rules = nullptr;

	HRESULT ret = (*fw)->get_Rules(&rules);

	if (SUCCEEDED(ret)) {

		IUnknown* unknow = nullptr;
		ret = rules->get__NewEnum(&unknow);

		if ((unknow != nullptr) && (SUCCEEDED(ret))) {

			IEnumVARIANT* iterator = nullptr;
			ret = unknow->QueryInterface(__uuidof(IEnumVARIANT), reinterpret_cast<void**>(&iterator));

			if (SUCCEEDED(ret)) {

				CComVariant var;
				ULONG fetched = 0;
				INetFwRule* rule = nullptr;

				while (ret != S_FALSE) {

					   var.Clear();
					   ret = iterator->Next(1, &var, &fetched);

					   if (ret != S_FALSE) {

						   ret = var.ChangeType(VT_DISPATCH);

						   if (!SUCCEEDED(ret)) {
							   break;
						   }

						   ret = (V_DISPATCH(&var))->QueryInterface(__uuidof(INetFwRule), reinterpret_cast<void**>(&rule));

						   if (!SUCCEEDED(ret)) {
							   break;
						   }

						   if (parse(rule, raddr_pattern, rport_pattern, lport_pattern,  
							         protocol, direction, action, profile) == -1) {

							   throw std::runtime_error("Juche::Firewall::Firewall::rules()->parse FAIL");

						   }

					   }

				}

			}

		}

	}

	return ret;

}

/// <summary>
/// Konstruktor klasy Firewall ( RAII ).
/// Wszystkie parametry maj� zdefiniowane warto�ci domy�lne.
/// </summary>
/// <param name="com_init">
/// Warto�� true lub false, na jej podstawie wywo�ywana jest 
/// funkcja CoInitializeEx.
/// </param>
/// <param name="raddr_pattern">
/// Warto�� przekazywana do metdo rules i parse.
/// </param>
/// <param name="rport_pattern">
/// Warto�� przekazywana do metdo rules i parse.
/// </param>
/// <param name="lport_pattern">
/// Warto�� przekazywana do metdo rules i parse.
/// </param>
/// <param name="protocol">
/// Warto�� przekazywana do metdo rules i parse.
/// </param>
/// <param name="direction">
/// Warto�� przekazywana do metdo rules i parse.
/// </param>
/// <param name="action">
/// Warto�� przekazywana do metdo rules i parse.
/// </param>
/// <param name="profile">
/// Warto�� przekazywana do metdo rules i parse.
/// </param>
/// <returns>none</returns>
Juche::Firewall::Firewall::Firewall(bool com_init,
	const string_t& raddr_pattern,
	const string_t& rport_pattern,
	const string_t& lport_pattern,
	NET_FW_IP_PROTOCOL protocol,
	TrafficType direction,
	NET_FW_ACTION action,
	NET_FW_PROFILE_TYPE2 profile) noexcept(false) : com_uninit(com_init), policies(0), 
	                                                fw(std::make_shared<INetFwPolicy2*>(nullptr)),
	                                                aa(new AuthorizedApplications(true, NET_FW_SCOPE_ALL, raddr_pattern)) {

	if (com_init == true) {
		CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	}

	CoCreateInstance(__uuidof(NetFwPolicy2), NULL,
		CLSCTX_INPROC_SERVER, __uuidof(INetFwPolicy2),
		reinterpret_cast<LPVOID*>(&(*fw)));
	
	if(!SUCCEEDED(rules(raddr_pattern, rport_pattern, lport_pattern, protocol, direction, action, profile))) {
	    throw std::runtime_error("Juche::Firewall::Firewall::Firewall->constructor()->rules() FAIL");
    }

}

/// <summary>
/// Weryfikuje czy dla wskazanego profilu dozwolone s� po��czenia wychodz�ce 
/// (domy�lnie s�).
/// </summary>
/// <param name="profile">
/// Warto�� okre�laj�ca profil, aktualnie profil aktualny.
/// Docelowo prywatny, domenowy, publiczny 
/// </param>
/// <returns>
/// -1 = error (mo�na rzuci� wyj�tkiem)
///  0 = fa�sz (po��czenia wychodz�ce s� zablokowane)
///  1 = prawda (po��czenia wychodz�ce s� dozwolone)
/// </returns>
int 
Juche::Firewall::Firewall::are_outbound_connection_allowed(NET_FW_PROFILE_TYPE2 profile) const noexcept(false) {

	/*
	VARIANT_BOOL enabled;
	if (!SUCCEEDED((*fw)->get_FirewallEnabled(profile,&enabled))) {
		return -1;
	}

	if (enabled == VARIANT_FALSE) {
		return 1;
	} */

	int res = enabled(profile); 
	if (res == -1) {
		return -1;
	}

	if (res == 0) {
		return 1;
	}

	NET_FW_ACTION action;
	if (!SUCCEEDED((*fw)->get_DefaultOutboundAction(profile, &action))) {
		return -1;
	}

	if (action != NET_FW_ACTION_BLOCK) {
		return 1;
	}

	return 0;

}

/// <summary>
/// Sprawdza czy zapora jest w��czona dla okre�lonego profilu. 
/// Sprawdzenie jest wykonywane na poziomie LPO (polityk lokalnych, nie GPO).
/// </summary>
/// <param name="profile">profil (domenowy, lokalny, publiczny). Domy�lnie NET_FW_PROFILE2_PUBLIC</param>
/// <returns>
/// 1 = zapora w��czona
/// 0 = zapora wy��czona
/// -1 = error
/// </returns>
int 
Juche::Firewall::Firewall::enabled(NET_FW_PROFILE_TYPE2 profile) const noexcept(false) {

	VARIANT_BOOL enabled;
	if (!SUCCEEDED((*fw)->get_FirewallEnabled(profile, &enabled))) {
		return -1;
	}

	if (enabled == VARIANT_TRUE) {
		return 1;
	}

	return 0;

}

/// <summary>
/// Od�wie�a informacje o regu�ach FW na bazie przekazanych parametr�w.
/// Np. Instancja Firewall mo�e zosta� utworzona dla regu� okre�lonego protoko�u, kierunku po��cze� etc.
/// Metoda refres pozwala zmieni� te parametry, dzi�ki temu mo�emy sprawdzi� np. regu�y wychodz�ce a p�niej 
/// za pomoc� refresh pobra� i sprawdzi� regu�y dla ruchu przychodz�cego.
/// </summary>
/// <param name="raddr_pattern">Parametr przekazywany do rules oraz konstruktora AuthorizedApplications.</param>
/// <param name="rport_pattern">Parametr przekazywany do rules.</param>
/// <param name="lport_pattern">Parametr przekazywany do rules.</param>
/// <param name="protocol">Parametr przekazywany do rules.</param>
/// <param name="direction">Parametr przekazywany do rules.</param>
/// <param name="action">Parametr przekazywany do rules.</param>
/// <param name="profile">Parametr przekazywany do rules.</param>
/// <returns>
/// true = wywo�anie metody powiod�o si�
/// false = wywo�anie metody nie powiod�o si�
/// </returns>
bool 
Juche::Firewall::Firewall::refresh(const string_t& raddr_pattern,
	                               const string_t& rport_pattern,
	                               const string_t& lport_pattern,
	                               NET_FW_IP_PROTOCOL protocol,
	                               TrafficType direction,
	                               NET_FW_ACTION action,
	                               NET_FW_PROFILE_TYPE2 profile) noexcept(false) {


	raw_release<Firewall>(this);

	policies.erase(policies.begin(), policies.end());

#ifdef __J_DEBUG__
	Juche::Helpers::DebugConsole* dbg = Juche::Helpers::DebugConsole::instance();
	dbg->write(TEXT("Juche::Firewall::Firewall::refresh()->erase(policies) OK, policies->size: %d\r\n"), policies.size());
#endif

	if (!SUCCEEDED(rules(raddr_pattern, rport_pattern, lport_pattern,
		protocol, direction, action, profile))) {
		return false;
	}
   
	if (aa != nullptr) {
		delete aa;
		aa = std::move(new AuthorizedApplications(true, NET_FW_SCOPE_ALL, raddr_pattern));
	}

#ifdef __J_DEBUG__
	dbg->write(TEXT("Juche::Firewall::Firewall::refresh() OK, policies->size: %d\r\n"), policies.size());
#endif
	
	return true;

}

/// <summary>
/// Sprawdza czy aplikacja kt�rej �cie�ka definiowana jest parametrem name 
/// znajduje si� na li�cie wyj�tk�w i opcjonalnie autoryzowanych aplikacji zapory ogniowej.
/// </summary>
/// <param name="name">�cie�ka do aplikacji kt�r� chcemy sprawdzi�.</param>
/// <param name="include_authorized_app">
/// W przypadku warto�ci true metoda zweryfikuje czy aplikacja znajduje
/// si� r�wnie� na li�cie autoryzowanych aplikacji w przypadku gdy nie znajduje si� na li�cie wyj�tk�w.
/// W przypadku warto�ci false autoryzowane aplikacje nie b�d� sprawdzane.
/// </param>
/// <returns>
/// true  = aplikacja znajduje si� na li�cie 
/// false = aplikacji nie ma na li�cie 
/// </returns>
bool
Juche::Firewall::Firewall::in_exception(const string_t& name, bool include_authorized_app) const noexcept(false) {


	for(const Settings<INetFwRule>& item : policies) {

		if (item.app_path == name) {
			return true;
		}

	}
	
	if (include_authorized_app == true) {

		int ret = aa->is_authorized(name);

		if (ret == -1) {
			throw std::runtime_error("Juche::Firewall::Firewall::in_exception()->aa->is_authorized FAIL");
		}

		if (ret == 1) {
			return true;
		}

	}
	
	return false;

}

/// <summary>
/// Przypisanie przenosz�ce.
/// </summary>
/// <param name="applications">
/// Referencja do r-warto�ci typu AuthorizedApplications
/// (obiekt tymczasowy) 
/// </param>
/// <returns>referencja do samego siebie</returns>
Juche::Firewall::AuthorizedApplications&
Juche::Firewall::AuthorizedApplications::operator=(AuthorizedApplications&& applications) noexcept(false) {   

	if (this != &applications) {

		aa = nullptr;

		aa.swap(applications.aa);              

		com_uninit = applications.com_uninit;

		std::swap(policies, applications.policies);    

	}

	return *this;

}

/// <summary>
/// Konstruktor przenoszenia.
/// </summary>
/// <param name="applications">
/// Referencja do r-warto�ci typu AuthorizedApplications
/// (obiekt tymczasowy) 
/// </param>
Juche::Firewall::AuthorizedApplications::AuthorizedApplications(AuthorizedApplications&& applications) noexcept(false) : policies(0), 
                                                                                                                         aa(std::make_shared<INetFwAuthorizedApplications*>(nullptr)) {

	*this = std::move(applications);

}

/// <summary>
/// Inicjalizuje wska�nik (sk�adow�) INetFwAuthorizedApplications aa.
/// </summary>
/// <param name="profile">
/// Warto�� definiuje profil firewall, aktualnie wspierany jest tylko aktualny profil.
/// </param>
/// <returns>
/// Warto�� HRESULT zwr�cona przez ostatnie niepoprawne wywo�anie metody COM lub
/// S_OK (w przypadku kiedy podczas wywo�ywania metody nie pojawi� si� �aden b��d).
/// </returns>
HRESULT 
Juche::Firewall::AuthorizedApplications::aa_instance(NET_FW_PROFILE_TYPE profile) noexcept {

	INetFwMgr* fw_mgr;

#ifdef __J_DEBUG__
	Juche::Helpers::DebugConsole* dbg = Juche::Helpers::DebugConsole::instance();
#endif
	
	HRESULT ret = CoCreateInstance(__uuidof(NetFwMgr), NULL,
		                           CLSCTX_INPROC_SERVER, __uuidof(INetFwMgr),
		                           reinterpret_cast<LPVOID*>(&fw_mgr));

	if (!FAILED(ret)) {

		INetFwPolicy* fw_policy;
		ret = fw_mgr->get_LocalPolicy(&fw_policy);

#ifdef __J_DEBUG__
		dbg->write(TEXT("Juche::Firewall::AuthorizedApplications::aa_instance()->CoCreateInstance OK\r\n"));
#endif

		if (ret == S_OK) {

			INetFwProfile* fw_profile;
			ret = fw_policy->GetProfileByType(profile, &fw_profile);

			if (ret == S_OK) {

				INetFwAuthorizedApplications* aa_ptr;
				ret = fw_profile->get_AuthorizedApplications(&aa_ptr);

				if (ret == S_OK) {
					aa = std::make_shared<INetFwAuthorizedApplications*>(aa_ptr);
				}

			}

			fw_policy->Release();

		}

		fw_mgr->Release();

	}

	return ret;

}


/// <summary>
/// Weryfikuje regu�e autoryzowanych aplikacji pod k�tem zgodno�ci z oczekiwanym wzorcem.
/// </summary>
/// <param name="app">Wska�nik do INetFwAuthorizedApplication reprezentuj�cy regu�e autoryzowanych aplikacji.</param>
/// <param name="scope">Zakres kt�rego dotyczy regu�a</param>
/// <param name="raddr_pattern">
/// Wyra�enie reguralne do por�wnania z warto�ci� uzyskan� przez wywo�anie get_RemoteAddresses kt�ra zwaraca
/// zdalne adresy sieciowe z kt�rymi mo�na nawi�za� ��czno�� sieciow�.
/// </param>
/// <returns>
/// -1 = error
///  0 = fa�sz, regu�a nie jest zgodna z oczekiwaniami
///  1 = true, regu�a spe�nia oczekiwania
/// </returns>
int 
Juche::Firewall::AuthorizedApplications::parse(INetFwAuthorizedApplication* app, 
	                                           NET_FW_SCOPE scope,
	                                           const string_t& raddr_pattern) noexcept(false) {


	VARIANT_BOOL enabled;
	if (!SUCCEEDED(app->get_Enabled(&enabled))) {
		return -1;
	}

	if (enabled != VARIANT_TRUE) {
		return 0;
	}

	NET_FW_IP_VERSION ip_information;
	if (!SUCCEEDED(app->get_IpVersion(&ip_information))) {
		return -1;
	}

	NET_FW_SCOPE fw_scope;
	if (!SUCCEEDED(app->get_Scope(&fw_scope))) {
		return -1;
	}

	if (fw_scope != scope) {
		return 0;
	}

	BSTR r_address;
	if (!SUCCEEDED(app->get_RemoteAddresses(&r_address))) { 
		return -1;
	}

	BSTR rule_name;                                         
	if (!SUCCEEDED(app->get_Name(&rule_name))) {
		return -1;
	}

	BSTR image_filename;                                   
	if (!SUCCEEDED(app->get_ProcessImageFileName(&image_filename))) {
		return -1;
	}

	regex_t regexp(raddr_pattern);

#ifndef UNICODE
	std::unique_ptr<char[]> tmp_address(Juche::Helpers::U2A(r_address));
	std::string n_address = tmp_address.get();

	std::unique_ptr<char[]> tmp_rule_name(Juche::Helpers::U2A(rule_name));      
	std::string n_rulename = tmp_rule_name.get();

	std::unique_ptr<char[]> tmp_file_name(Juche::Helpers::U2A(image_filename)); 
	std::string n_filename = tmp_file_name.get();
	//TODO: konwersja rule_name i image_filename
#else
	std::wstring n_address = r_address;
	std::wstring n_rulename = rule_name;
	std::wstring n_filename = image_filename;
#endif

	if (!std::regex_match(n_address, regexp)) {
		return 0;
	}
	
	Settings<INetFwAuthorizedApplication> info;
	
	info.direction = TrafficType::ANY;
	info.raw = app;
	info.ip_version = ip_information;
	info.app_path = n_filename;
	info.name = n_rulename;

#ifdef __J_DEBUG__
	Juche::Helpers::DebugConsole* dbg = Juche::Helpers::DebugConsole::instance();
	dbg->write(TEXT("Juche::Firewall::AuthorizedApplications::parse()\r\n"
		            "Rule enabled : 0x%x\r\n"
		            "info.ip_version: 0x%x dec(%d)\r\n"
		            "fw_scope: 0x%x dec(%d)\r\n"
		            "n_address: %s\r\n"
		            "info.name: %s\r\n"
		            "info.app_path: %s\r\n"
		            "__________________________________________________\r\n"
	), enabled, info.ip_version, info.ip_version, fw_scope, fw_scope, n_address.c_str(), info.name.c_str(), info.app_path.c_str());
#endif
	
	policies.push_back(info);

	return 1;

}


/// <summary>
/// Tworzy iterator przy pomocy kt�rego uzysykujemy dost�p do ka�dej regu�y reprezentowanej przez wska�nik 
/// do INetFwAuthorizedApplication.
/// </summary>
/// <param name="scope">
/// Parametr przekazywany do wywo�ania parse.
/// </param>
/// <param name="raddr_pattern">
/// Parametr przekazywany do wywo�ania parse.</param>
/// <returns>
/// Warto�� HRESULT zwr�cona przez ostatnie nieporawne wywo�anie metody COM lub
/// S_OK (w przypadku kiedy podczas wywo�ywania metody nie pojawi� si� �aden b��d).
/// </returns>
HRESULT 
Juche::Firewall::AuthorizedApplications::rules(NET_FW_SCOPE scope, const string_t& raddr_pattern) noexcept(false) {

	long count;
	HRESULT ret = (*aa)->get_Count(&count);

	if (SUCCEEDED(ret)) {

		if (count > 0) {

			IUnknown* unknow = nullptr;
			ret = (*aa)->get__NewEnum(&unknow);

			if ((unknow != nullptr) && (SUCCEEDED(ret))) {

				IEnumVARIANT* iterator;
				ret = unknow->QueryInterface(__uuidof(IEnumVARIANT), reinterpret_cast<void**>(&iterator));

				if (SUCCEEDED(ret)) {

					CComVariant var;
					ULONG fetched = 0;
					INetFwAuthorizedApplication* app = nullptr;

					while (ret != S_FALSE) {

						var.Clear();
						ret = iterator->Next(1, &var, &fetched);

						if (ret != S_FALSE) {

							ret = var.ChangeType(VT_DISPATCH);
							if (!SUCCEEDED(ret)) {
								break;
							}

							ret = (V_DISPATCH(&var))->QueryInterface(__uuidof(INetFwAuthorizedApplication), reinterpret_cast<void**>(&app));
							if (!SUCCEEDED(ret)) {
								break;
							}

					        if (parse(app, scope, raddr_pattern) == -1) {
								throw std::runtime_error("Juche::Firewall::AuthorizedApplications::rules()->parse FAIL");
							}

						}

					}

				}


			}

		}

	}

	return ret;

}

/// <summary>
/// Sprawdza czy aplikacja znajduje si�
/// na li�cie autoryzowanych aplikacji.
/// </summary>
/// <param name="app">�cie�ka do aplikacji</param>
/// <param name="scope">Zakres</param>
/// <param name="raddr_pattern">Wyra�enie reguralne do por�wnania zdalnego adresu sieciowego</param>
/// <returns>
///  1 = prawda
///  0 = falsz
/// -1 = error
/// </returns>
int
Juche::Firewall::AuthorizedApplications::is_authorized(const string_t& app, NET_FW_SCOPE scope, const string_t& raddr_pattern) noexcept(false) {

	int ret = -1;
	
	for (const auto& item : policies) {
		 
		if (item.app_path == app) {
			return 1;
		}

	}

#ifndef UNICODE 
	std::unique_ptr<wchar[]> tmp_app(Juche::Helpers::A2U(app));
	std::wstring n_app = tmp_app.get();
#else
	std::wstring n_app = app;
#endif

	BSTR ole_app = SysAllocStringLen(n_app.c_str(), static_cast<UINT>(n_app.size()));

	if (ole_app != nullptr) {

		INetFwAuthorizedApplication* authed_app = nullptr;

		if (!SUCCEEDED((*aa)->Item(ole_app, &authed_app))) {

			ret = 0;

		} else {

		  VARIANT_BOOL enabled;

		  if (SUCCEEDED(authed_app->get_Enabled(&enabled))) {
				
			 if (enabled != VARIANT_TRUE) {
					
				 ret = 0;

			 } else {

				ret = parse(authed_app, scope, raddr_pattern);

			 }
		  }
			
		}

		SysFreeString(ole_app);

	}

	return ret;

}

/// <summary>
/// Konstruktor AuthorizedApplications
/// </summary>
/// <param name="com_init">Decyduje o wywo�aniu CoInitializeEx oraz CoUninitialize</param>
/// <param name="scope">Zakres</param>
/// <param name="raddr_pattern">Wzorzec dla wyra�enia reguralnego zdalnych adres�w sieciowych</param>
/// <returns>brak</returns>
Juche::Firewall::AuthorizedApplications::AuthorizedApplications(bool com_init, NET_FW_SCOPE scope, 
	                                                            const string_t& raddr_pattern) noexcept(false) :
	                                                            com_uninit(com_init), 
	                                                            aa(std::make_shared<INetFwAuthorizedApplications*>(nullptr)),
	                                                            policies(0) {

	if (com_init == true) {
		CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	}

	if (!SUCCEEDED(aa_instance())) {
		throw std::runtime_error("Juche::Firewall::AuthorizedApplications->constructor()->aa_instance FAIL");
	}

	rules(scope, raddr_pattern);

}

/// <summary>
/// AuthorizedApplications destruktor
/// </summary>
Juche::Firewall::AuthorizedApplications::~AuthorizedApplications() {

#ifdef __J_DEBUG__
	Juche::Helpers::DebugConsole* dbg = Juche::Helpers::DebugConsole::instance();
	dbg->write(TEXT("Juche::Firewall::AuthorizedApplications::~AuthorizedApplications() DESTRUCTOR policies.size() %d\r\n"), policies.size());
#endif

	raw_release<AuthorizedApplications>(this);

	if (policies.size() > 0) {

		policies.erase(policies.begin(), policies.end());

	}

	if (aa.use_count() == 1) {

		if ((*aa) != nullptr) {

			(*aa)->Release();

		}

	}

	if (com_uninit == true) {

		CoUninitialize();

	}


}

/// <summary>
/// Konstruktor przenoszenia Firewall
/// </summary>
/// <param name="other">R-Referencja do Firewall</param>
/// <returns>nic</returns>
Juche::Firewall::Firewall::Firewall(Firewall&& other) noexcept(false) : aa(nullptr), policies(0), 
                                                                                     fw(std::make_shared<INetFwPolicy2*>(nullptr)) {

	*this = std::move(other);

}

/// <summary>
/// Przypisanie przenosz�ce 
/// </summary>
/// <param name="other">R-Referencja do Firewall</param>
/// <returns>none</returns>
Juche::Firewall::Firewall&
Juche::Firewall::Firewall::operator=(Firewall&& other) noexcept(false) {

	if (this != &other) {

		com_uninit = other.com_uninit;

		fw.swap(other.fw);

		std::swap(aa, other.aa);

		std::swap(policies, other.policies);

	}

	return *this;

}