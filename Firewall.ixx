////////////////////////////////////////////////////////////////////////////
///21/04/2022 17:12
///Modu� umo�liwia pobranie regu� zapory sieciowej oraz sprawdzenie 
///na podstawie �cie�ki do pliku czy dana aplikacja ma mo�liwo�� odbierania 
///lub nawi�zywania po��cze� z okre�lonych i do okre�lonych adres�w IP na okre�lonych portach sieciowych.
///Klasy maj� mo�liwo�� relatywnie �atwego rozszerzenia o mo�liwo�ci edycji i definiowania 
///nowych regu� zapory sieciowej.
////////////////////////////////////////////////////////////////////////////

module;

#include <string>
#include <vector>
#include <netfw.h>

#include "Environment.h"

export module Firewall;

namespace Juche {

	namespace Firewall {

		export {

			enum class TrafficType : unsigned char {
				 OUTBOUND,
				 INBOUND,
				 ANY
			};

			template<typename T>
			struct Settings {

				TrafficType direction;
				/// <summary>
				/// raw : Gdyby�my kiedy� chcieli rozszerzy� to arcydzie�o sztuki nowo�ytnej o mo�liwo�� edycji zasad.
				/// </summary>
				T* raw;        
				union {
					NET_FW_IP_PROTOCOL protocol;
					NET_FW_IP_VERSION  ip_version;
				};
				string_t    app_path;
				string_t    name;

			};

			/// <summary>
			/// Metoda pomocnicza, zwalnia raw 
			/// </summary>
			/// <typeparam name="T">Typ wskaznikowy do instacji klasy z kt�r� si� przyja�ni</typeparam>
			/// <param name="ref">this</param>
			/// <returns>void</returns>
			template<typename T>
			inline void raw_release(T ref) noexcept {
				 
				size_t sz = ref->policies.size();

				for (size_t i = 0; i < sz; i++) {

					if (ref->policies[i].raw != nullptr) {

						ref->policies[i].raw->Release();
						ref->policies[i].raw = nullptr;

				    }

				}

			}

			class AuthorizedApplications {  

				  bool com_uninit;

				  /// <summary>
				  /// shared_ptr bo COM.
				  /// </summary>
				  std::shared_ptr<INetFwAuthorizedApplications*> aa;

				  HRESULT aa_instance(NET_FW_PROFILE_TYPE profile = NET_FW_PROFILE_CURRENT) noexcept;

				  template<typename T>
				  friend void raw_release(T ptr) noexcept;

			protected:

				  HRESULT rules(NET_FW_SCOPE scope, const string_t& raddr_pattern) noexcept(false);

				  int parse(INetFwAuthorizedApplication* app, NET_FW_SCOPE scope,
					        const string_t& raddr_pattern) noexcept(false);

				  std::vector<Settings<INetFwAuthorizedApplication>> policies;

			public:

				  const 
				  Settings<INetFwAuthorizedApplication>* is_authorized(const string_t& app, 
				                                                       NET_FW_SCOPE scope = NET_FW_SCOPE_ALL,
					                                                   const string_t& raddr_pattern = TEXT("\\*")) noexcept(false);

				  explicit AuthorizedApplications(bool com_init = true,
					                              NET_FW_SCOPE scope = NET_FW_SCOPE_ALL,
					                              const string_t& raddr_pattern = TEXT("\\*")
					                              ) noexcept(false);

				  virtual ~AuthorizedApplications();

				  /// <summary>
				  /// Kopiowanie jest wy��czone z uwagi na wska�niki do obiekt�w COM,
				  /// kt�re s� sk�adowymi klasy. Nie umiem i chyba si� w og�le nieda
				  /// skopiowa� ich w taki spos�b aby nie tworzy� "spl�tanych" obiekt�w.
				  /// (chocia� w sumie to nie jestem pewien czy one s� "spl�tane", ale zostaje jak jest, nie chce mi si� sprawdza�)
				  /// </summary>
				  AuthorizedApplications(const AuthorizedApplications& applications) = delete;
				  AuthorizedApplications& operator=(AuthorizedApplications& applications) = delete;

				  /// <summary>
				  /// Przenoszenie jest mo�liwe.
				  /// </summary>
				  AuthorizedApplications(AuthorizedApplications&& applications) noexcept(false);
				  AuthorizedApplications& operator=(AuthorizedApplications&& applications) noexcept(false);  

			};

			class Firewall {            

				  bool com_uninit;

				  /// <summary>
				  /// share_ptr COM
				  /// </summary>
				  std::shared_ptr<INetFwPolicy2*> fw;

				  AuthorizedApplications* aa; 

				  template<typename T>
				  friend void raw_release(T ptr) noexcept;

			protected:

				  std::vector<Settings<INetFwRule>> policies;

				  int parse(INetFwRule* rule, 
					        const string_t& raddr_pattern, 
					        const string_t& rport_pattern,
					        const string_t& lport_pattern,
					        NET_FW_IP_PROTOCOL protocol, TrafficType direction,
					        NET_FW_ACTION action, NET_FW_PROFILE_TYPE2 profile) noexcept(false);

				  HRESULT rules(const string_t& raddr_pattern, 
					            const string_t& rport_pattern,
					            const string_t& lport_pattern,
					            NET_FW_IP_PROTOCOL protocol, TrafficType direction, 
					            NET_FW_ACTION action, NET_FW_PROFILE_TYPE2 profile) noexcept(false);

			public:

				  Firewall(Firewall&& other) noexcept(false);              
				  Firewall& operator=(Firewall&& other) noexcept(false);  

				  /// <summary>
				  /// Nie kopiujemy bo COM ;P
				  /// </summary>
				  /// <param name="">ref Firewall</param>
				  Firewall(const Firewall&) = delete;
				  Firewall& operator=(Firewall&) = delete;
				  
				  explicit Firewall(bool com_init = false,
					                const string_t& raddr_pattern = TEXT("\\*"),
					                const string_t& rport_pattern = TEXT("\\*"),
					                const string_t& lport_pattern = TEXT("\\*"),
					                NET_FW_IP_PROTOCOL protocol = NET_FW_IP_PROTOCOL_TCP, 
					                TrafficType direction = TrafficType::OUTBOUND,
					                NET_FW_ACTION action = NET_FW_ACTION_ALLOW,
					                NET_FW_PROFILE_TYPE2 profile = NET_FW_PROFILE2_PUBLIC) noexcept(false);
				   
				  virtual ~Firewall();

				  int are_outbound_connection_allowed(NET_FW_PROFILE_TYPE2 profile = NET_FW_PROFILE2_PUBLIC) const noexcept(false);

				  bool refresh(const string_t& raddr_pattern = TEXT("\\*"),
					           const string_t& rport_pattern = TEXT("\\*"),
					           const string_t& lport_pattern = TEXT("\\*"),
					           NET_FW_IP_PROTOCOL protocol = NET_FW_IP_PROTOCOL_TCP,
					           TrafficType direction = TrafficType::OUTBOUND,
					           NET_FW_ACTION action = NET_FW_ACTION_ALLOW,
					           NET_FW_PROFILE_TYPE2 profile = NET_FW_PROFILE2_PUBLIC) noexcept(false);


				  bool in_exception(const string_t& name, bool include_authorized_app = true) const noexcept(false);

			};

		}

	}

}