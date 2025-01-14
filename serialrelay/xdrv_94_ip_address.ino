/*
  xdrv_92_ipaddress.ino - Fixed IP address management
  
  Copyright (C) 2020  Nicolas Bernaerts
    20/04/2021 - v1.0 - Creation 
    22/04/2021 - v1.1 - Merge IPAddress and InfoJson 

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*************************************************\
 *                IP Address
\*************************************************/

#ifdef USE_IPADDRESS

#define XDRV_94                   94

// commands
#define D_CMND_IPADDRESS_ADDR     "addr"
#define D_CMND_IPADDRESS_GATEWAY  "gway"
#define D_CMND_IPADDRESS_NETMASK  "nmsk"
#define D_CMND_IPADDRESS_DNS      "dns"

// web URL
const char D_IPADDRESS_PAGE_CONFIG[] PROGMEM = "/ip";
const char D_IPADDRESS_PAGE_JSON[]   PROGMEM = "/info.json";

// dialog strings
const char D_IPADDRESS_CONFIGURE[] PROGMEM = "Configure IP";
const char D_IPADDRESS_IP[]        PROGMEM = "IP";
const char D_IPADDRESS_ADDRESS[]   PROGMEM = "Address <small>(0.0.0.0 for DHCP)</small>";
const char D_IPADDRESS_GATEWAY[]   PROGMEM = "Gateway";
const char D_IPADDRESS_NETMASK[]   PROGMEM = "Netmask";
const char D_IPADDRESS_DNS[]       PROGMEM = "DNS";

// offloading commands
enum IPAddressCommands { CMND_IPADDRESS_ADDR, CMND_IPADDRESS_GATEWAY, CMND_IPADDRESS_NETMASK, CMND_IPADDRESS_DNS };
const char kIPAddressCommands[]    PROGMEM = "addr|gateway|netmask|dns";

// constant strings
const char D_IPADDRESS_FIELDSET_START[] PROGMEM = "<p><fieldset><legend><b>&nbsp;%s&nbsp;</b></legend>\n";
const char D_IPADDRESS_FIELDSET_STOP[]  PROGMEM = "</fieldset></p><br>\n";
const char D_IPADDRESS_FIELD_INPUT[]    PROGMEM = "<p>%s<br><input type='text' name='%s' value='%_I' minlength='7' maxlength='15'></p>\n";

/**************************************************\
 *                  Functions
\**************************************************/

// Show JSON status (for MQTT)
void IPAddressShowJSON ()
{
  // append JSON to MQTT message
  ResponseAppend_P ( PSTR ("\"Wifi\":{\"IP\":\"%s\",\"MAC\":\"%s\",\"SSID\":\"%s\",\"Quality\":\"%d\",\"Hostname\":\"%s\"}"), WiFi.localIP ().toString ().c_str (), WiFi.macAddress ().c_str (), SettingsText (SET_STASSID1 + Settings.sta_active), WifiGetRssiAsQuality (WiFi.RSSI ()), TasmotaGlobal.hostname);
}

/***********************************************\
 *                    Web
\***********************************************/

#ifdef USE_WEBSERVER

// IP address configuration page button
void IPAddressWebConfigButton ()
{
  WSContentSend_P (PSTR ("<p><form action='%s' method='get'><button>%s</button></form></p>"), D_IPADDRESS_PAGE_CONFIG, D_IPADDRESS_CONFIGURE);
}

// IP address configuration web page
void IPAddressWebPageConfigure ()
{
  bool     updated = false;
  uint32_t value;
  char     str_argument[32];

  // if access not allowed, close
  if (!HttpCheckPriviledgedAccess()) return;

  // page comes from save button on configuration page
  if (Webserver->hasArg ("save"))
  {
    // device ip address
    WebGetArg (D_CMND_IPADDRESS_ADDR, str_argument, sizeof (str_argument));
    if (strlen (str_argument) > 0)
    {
      ParseIPv4 (&value, str_argument);
      updated |= (value != Settings.ipv4_address[0]);
      Settings.ipv4_address[0] = value;
    }

    // gateway
    WebGetArg (D_CMND_IPADDRESS_GATEWAY, str_argument, sizeof (str_argument));
    if (strlen (str_argument) > 0)
    {
      ParseIPv4 (&value, str_argument);
      updated |= (value != Settings.ipv4_address[1]);
      Settings.ipv4_address[1] = value;
    }

    // net mask
    WebGetArg (D_CMND_IPADDRESS_NETMASK, str_argument, sizeof (str_argument));
    if (strlen (str_argument) > 0)
    {
      ParseIPv4 (&value, str_argument);
      updated |= (value != Settings.ipv4_address[2]);
      Settings.ipv4_address[2] = value;
    }

    // gateway
    WebGetArg (D_CMND_IPADDRESS_DNS, str_argument, sizeof (str_argument));
    if (strlen (str_argument) > 0)
    {
      ParseIPv4 (&value, str_argument);
      updated |= (value != Settings.ipv4_address[3]);
      Settings.ipv4_address[3] = value;
    }

    // if data have been updated, restart
    if (updated) WebRestart (1);
  }

  // beginning of form
  WSContentStart_P (D_IPADDRESS_CONFIGURE);
  WSContentSendStyle ();
  WSContentSend_P (PSTR ("<form method='get' action='%s'>\n"), D_IPADDRESS_PAGE_CONFIG);

  // IP address section  
  // ---------------------
  WSContentSend_P (D_IPADDRESS_FIELDSET_START, D_IPADDRESS_IP);

  WSContentSend_P (D_IPADDRESS_FIELD_INPUT, D_IPADDRESS_ADDRESS, PSTR (D_CMND_IPADDRESS_ADDR),    Settings.ipv4_address[0]);
  WSContentSend_P (D_IPADDRESS_FIELD_INPUT, D_IPADDRESS_GATEWAY, PSTR (D_CMND_IPADDRESS_GATEWAY), Settings.ipv4_address[1]);
  WSContentSend_P (D_IPADDRESS_FIELD_INPUT, D_IPADDRESS_NETMASK, PSTR (D_CMND_IPADDRESS_NETMASK), Settings.ipv4_address[2]);
  WSContentSend_P (D_IPADDRESS_FIELD_INPUT, D_IPADDRESS_DNS,     PSTR (D_CMND_IPADDRESS_DNS),     Settings.ipv4_address[3]);

  WSContentSend_P (D_IPADDRESS_FIELDSET_STOP);

  // save button  
  // --------------
  WSContentSend_P (PSTR ("<p><button name='save' type='submit' class='button bgrn'>%s</button></p>\n"), D_SAVE);
  WSContentSend_P (PSTR ("</form>\n"));

  // configuration button
  WSContentSpaceButton(BUTTON_CONFIGURATION);

  // end of page
  WSContentStop();
}

// JSON version page
void IPAddressWebPageJSON ()
{
  char stopic[TOPSZ];

  // start of page
  WSContentBegin (200, CT_HTML);

  // {
  WSContentSend_P ( PSTR ("{"));

  // "Module":"NomDuModule","Version":"x.x.x","Uptime":"xxxxxx","Friendly name":"name",
  WSContentSend_P ( PSTR ("\"Module\":\"%s\",\"Version\":\"%s\",\"Uptime\":\"%s\",\"Name\":\"%s\","), ModuleName ().c_str (), TasmotaGlobal.version, GetUptime ().c_str (), SettingsText (SET_DEVICENAME));

#ifdef EXTENSION_NAME
  // "Extension":{"Type":"type","Version":"version","Author":"author"},
  WSContentSend_P ( PSTR ("\"Extension\":{\"Type\":\"%s\",\"Version\":\"%s\",\"Author\":\"%s\"},"), EXTENSION_NAME, EXTENSION_VERSION, EXTENSION_AUTHOR);
#endif

  // "MQTT":{"Host":"host","Port":"port","Topic":"topic"},
  WSContentSend_P ( PSTR ("\"MQTT\":{\"Host\":\"%s\",\"Port\":\"%d\",\"Topic\":\"%s\"},"), SettingsText (SET_MQTT_HOST), Settings.mqtt_port, GetTopic_P (stopic, CMND, TasmotaGlobal.mqtt_topic, ""));

  // "Wifi":{"IP":"xx.xx.xx.xx","MAC":"xx:xx:xx:xx:xx:xx:xx","SSID":"ssid,"Quality":"70","Hostname":"hostname"},
  WSContentSend_P ( PSTR ("\"Wifi\":{\"IP\":\"%s\",\"MAC\":\"%s\",\"SSID\":\"%s\",\"Quality\":\"%d\",\"Hostname\":\"%s\"}"), WiFi.localIP ().toString ().c_str (), WiFi.macAddress ().c_str (), SettingsText (SET_STASSID1 + Settings.sta_active), WifiGetRssiAsQuality (WiFi.RSSI ()), TasmotaGlobal.hostname);

  // }
  WSContentSend_P ( PSTR ("}"));

  // end of page
  WSContentEnd ();
}

#endif  // USE_WEBSERVER

/***********************************************************\
 *                      Interface
\***********************************************************/

bool Xdrv94 (uint8_t function)
{
  bool result = false;

  // main callback switch
  switch (function)
  { 
    case FUNC_JSON_APPEND:
      IPAddressShowJSON ();
      break;

#ifdef USE_WEBSERVER
    case FUNC_WEB_ADD_HANDLER:
      Webserver->on (FPSTR (D_IPADDRESS_PAGE_CONFIG), IPAddressWebPageConfigure);
      Webserver->on (FPSTR (D_IPADDRESS_PAGE_JSON), IPAddressWebPageJSON);
      break;
    case FUNC_WEB_ADD_BUTTON:
      IPAddressWebConfigButton ();
      break;
#endif  // USE_WEBSERVER

  }
  
  return result;
}

#endif // USE_IPADDRESS
