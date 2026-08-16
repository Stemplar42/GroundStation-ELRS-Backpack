#include <WiFi.h>
#include <WiFiUdp.h>
#include <TFT_eSPI.h>
#include <Preferences.h>
#include <math.h>
#include <qrcode.h>

// =====================================================
// GROUND STATION V2.5.1
// ESP32 + ExpressLRS Backpack WiFi + CRSF + ST7789 TFT
// Base V2.4.2 recuperada + brillo PWM corregido + reset EN
// =====================================================
//
// Basada en GroundStation V2.1 validada.
// Cambios V2.3:
// - HOME automatico al salir de modo WAIT.
// - Distancia y rumbo al HOME.
// - Flecha HOME tipo OSD en pantalla Horizonte.
// - Altitud y velocidad en pantalla Horizonte.
// - Mini brujula N/E/S/W en pantalla Horizonte.
// - Pantalla POWER con deteccion automatica de celdas y V/Celda por color.
// - Pantalla GPS con estilo mas tipo Garmin.
// - Plus Code en pantalla RECOVERY.
// - QR de recuperacion solo en TEL LOST / UDP LOST.
// - Conexion automatica al primer SSID ExpressLRS TX Backpack visible.
// - QR compatible con libreria Espressif qrcode.h.
// - Brillo PWM en BLK por GPIO25.
// - Pulsacion larga NEXT abre barra de brillo.
// - En modo brillo: NEXT sube, PREV baja, barra actualiza en vivo.
// - Boton RESET dedicado: EN a GND.
// - No se modifica Core CRSF / UDP / Backpack / estados.
// =====================================================

// =====================================================
// WIFI BACKPACK
// =====================================================

const char* BACKPACK_PREFIX = "ExpressLRS TX Backpack";
const char* password = "expresslrs";

String selectedBackpackSsid = "";

WiFiUDP udp;

#define UDP_PORT_LOCAL 14550
#define TIMEOUT_UDP_MS 2000
#define TIMEOUT_AIRCRAFT_TELEM_MS 2000

// =====================================================
// BOTONES
// =====================================================

#define BTN_PREV 32
#define BTN_NEXT 33

// =====================================================
// BRILLO PWM / RESET
// =====================================================
// GPIO25 debe ir al pin BLK / BL / BACKLIGHT del TFT.
// El boton RESET fisico va entre EN y GND. No requiere codigo.

#define PIN_BL 25
#define BL_PWM_FREQ 5000
#define BL_PWM_RES 8
#define BL_PWM_CH 0

const uint8_t BRIGHTNESS_LEVELS[] = {64, 128, 192, 255};
const int BRIGHTNESS_COUNT = 4;
int brightnessIndex = 3;
uint32_t brightnessOverlayUntilMs = 0;
Preferences prefs;

// =====================================================
// TFT / SPRITE
// =====================================================

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

const int SCREEN_W = 320;
const int SCREEN_H = 172;

const int SPR_W = 320;
const int SPR_H = 172;

// Horizonte artificial
const int CX = 160;
const int CY = 92;
const int R  = 68;

const float PITCH_PIXELS_PER_DEG = 2.0;

// =====================================================
// PANTALLAS
// =====================================================

enum PantallaGS {
  P_GENERAL = 0,
  P_HORIZONTE = 1,
  P_ENERGIA = 2,
  P_GPS = 3,
  P_DEBUG = 4
};

const int TOTAL_PANTALLAS = 5;
int pantallaActual = P_GENERAL;

// =====================================================
// ESTADOS
// =====================================================

bool tuvoPrimerUdp = false;
bool tuvoPrimerAircraftTelemetry = false;

// =====================================================
// HOME AUTOMATICO
// =====================================================

bool homeSet = false;
double homeLat = 0.0;
double homeLon = 0.0;
uint32_t homeSetMs = 0;

float homeDistanceM = 0.0;
float homeBearingDeg = 0.0;
float homeRelativeBearingDeg = 0.0;

bool lastGoodGpsSet = false;
double lastGoodLat = 0.0;
double lastGoodLon = 0.0;
int lastGoodAlt = 0;
uint32_t lastGoodGpsMs = 0;

int qrDrawX = 0;
int qrDrawY = 0;
int qrDrawScale = 2;

// =====================================================
// COLORES
// =====================================================

uint16_t SKY_COLOR;
uint16_t GROUND_COLOR;
uint16_t WHITE_COLOR;
uint16_t YELLOW_COLOR;
uint16_t CYAN_COLOR;
uint16_t ORANGE_COLOR;
uint16_t RED_COLOR;
uint16_t GREEN_COLOR;
uint16_t DARKGREY_COLOR;

// =====================================================
// ESTRUCTURA DE TELEMETRIA
// =====================================================

struct Telemetria {
  bool wifiConectado;
  bool recibeDatos;

  String modoVuelo;

  float voltaje;
  float corriente;
  uint32_t consumoMah;
  int bateriaRestante;

  int uplinkLQ;
  int downlinkLQ;

  int uplinkSNR;
  int downlinkSNR;

  int rfPowerEnum;
  String rfPowerTexto;

  float pitch;
  float roll;
  float yaw;

  double lat;
  double lon;
  float gpsSpeed;
  float gpsHeading;
  int gpsAltitude;
  int sats;

  float baroAltitude;
  float verticalSpeed;

  uint32_t paquetesUdp;
  uint32_t tramasMspV2;
  uint32_t tramasCrsf;
  uint32_t tramasTelemetriaAvion;
  uint32_t paquetesDesconocidos;

  uint32_t ultimoUdpMs;
  uint32_t ultimoCrsfMs;
  uint32_t ultimaTelemetriaAvionMs;
};

Telemetria tel;

// =====================================================
// INICIALIZACION TELEMETRIA
// =====================================================

void inicializarTelemetria()
{
  tel.wifiConectado = false;
  tel.recibeDatos = false;

  tel.modoVuelo = "---";

  tel.voltaje = 0.0;
  tel.corriente = 0.0;
  tel.consumoMah = 0;
  tel.bateriaRestante = -1;

  tel.uplinkLQ = -1;
  tel.downlinkLQ = -1;

  tel.uplinkSNR = 0;
  tel.downlinkSNR = 0;

  tel.rfPowerEnum = -1;
  tel.rfPowerTexto = "---";

  tel.pitch = 0.0;
  tel.roll = 0.0;
  tel.yaw = 0.0;

  tel.lat = 0.0;
  tel.lon = 0.0;
  tel.gpsSpeed = 0.0;
  tel.gpsHeading = 0.0;
  tel.gpsAltitude = 0;
  tel.sats = 0;

  tel.baroAltitude = 0.0;
  tel.verticalSpeed = 0.0;

  tel.paquetesUdp = 0;
  tel.tramasMspV2 = 0;
  tel.tramasCrsf = 0;
  tel.tramasTelemetriaAvion = 0;
  tel.paquetesDesconocidos = 0;

  tel.ultimoUdpMs = 0;
  tel.ultimoCrsfMs = 0;
  tel.ultimaTelemetriaAvionMs = 0;
}

// =====================================================
// FUNCIONES DE LECTURA BINARIA
// =====================================================

uint16_t readU16LE(const uint8_t* p)
{
  return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

uint16_t readU16BE(const uint8_t* p)
{
  return ((uint16_t)p[0] << 8) | (uint16_t)p[1];
}

int16_t readS16BE(const uint8_t* p)
{
  return (int16_t)readU16BE(p);
}

uint32_t readU24BE(const uint8_t* p)
{
  return ((uint32_t)p[0] << 16) |
         ((uint32_t)p[1] << 8)  |
         (uint32_t)p[2];
}

int32_t readS32BE(const uint8_t* p)
{
  uint32_t v =
    ((uint32_t)p[0] << 24) |
    ((uint32_t)p[1] << 16) |
    ((uint32_t)p[2] << 8)  |
    (uint32_t)p[3];

  return (int32_t)v;
}

// =====================================================
// MAPEO POTENCIA RF
// =====================================================

String mapRfPower(uint8_t rfPowerEnum)
{
  switch (rfPowerEnum) {
    case 0: return "10mW?";
    case 1: return "10/25?";
    case 2: return "25mW";
    case 3: return "50/100?";
    case 4: return "250mW?";
    case 5: return "500mW?";
    case 6: return "1000mW?";
    default: return "Enum " + String(rfPowerEnum);
  }
}

// =====================================================
// IDENTIFICACION DE TRAMAS DE TELEMETRIA DEL AVION
// =====================================================

bool esTramaTelemetriaAvion(uint8_t type)
{
  switch (type) {
    case 0x02: // GPS
    case 0x07: // VARIOMETER
    case 0x08: // BATTERY
    case 0x09: // BARO ALTITUDE
    case 0x1E: // ATTITUDE
    case 0x21: // FLIGHT MODE
      return true;

    default:
      return false;
  }
}

// =====================================================
// DECODIFICADORES CRSF
// =====================================================

void decodeFlightMode(const uint8_t* payload, int payloadLen)
{
  String modo = "";

  for (int i = 0; i < payloadLen; i++) {
    char c = (char)payload[i];

    if (c == 0) break;

    if (c >= 32 && c <= 126) {
      modo += c;
    }
  }

  if (modo.length() > 0) {
    tel.modoVuelo = modo;
  }
}

void decodeLinkStatistics(const uint8_t* p, int len)
{
  if (len < 10) return;

  uint8_t upLq    = p[2];
  int8_t  upSnr   = (int8_t)p[3];
  uint8_t rfPower = p[6];
  uint8_t downLq  = p[8];
  int8_t  downSnr = (int8_t)p[9];

  tel.uplinkLQ = upLq;
  tel.uplinkSNR = upSnr;

  tel.downlinkLQ = downLq;
  tel.downlinkSNR = downSnr;

  tel.rfPowerEnum = rfPower;
  tel.rfPowerTexto = mapRfPower(rfPower);
}

void decodeAttitude(const uint8_t* p, int len)
{
  if (len < 6) return;

  int16_t pitchRaw = readS16BE(p + 0);
  int16_t rollRaw  = readS16BE(p + 2);
  int16_t yawRaw   = readS16BE(p + 4);

  tel.pitch = -pitchRaw * 0.00572958f;
  tel.roll  = -rollRaw  * 0.00572958f;
  tel.yaw   = yawRaw   * 0.00572958f;
}

void decodeBattery(const uint8_t* p, int len)
{
  if (len < 8) return;

  uint16_t voltageRaw = readU16BE(p + 0);
  uint16_t currentRaw = readU16BE(p + 2);
  uint32_t capacityMah = readU24BE(p + 4);
  uint8_t remaining = p[7];

  tel.voltaje = voltageRaw / 10.0f;
  tel.corriente = currentRaw / 10.0f;
  tel.consumoMah = capacityMah;
  tel.bateriaRestante = remaining;
}

void decodeBaroAltitude(const uint8_t* p, int len)
{
  if (len < 2) return;

  uint16_t rawAlt = readU16BE(p);

  if (rawAlt == 0xFFFF) return;

  if ((rawAlt & 0x8000) == 0) {
    tel.baroAltitude = ((int32_t)rawAlt - 10000) / 10.0f;
  }
  else {
    tel.baroAltitude = rawAlt & 0x7FFF;
  }

  if (len >= 3) {
    int8_t vsRaw = (int8_t)p[2];
    tel.verticalSpeed = vsRaw * 0.03f;
  }
}

void decodeVariometer(const uint8_t* p, int len)
{
  if (len < 2) return;

  int16_t raw = readS16BE(p);
  tel.verticalSpeed = raw / 100.0f;
}

void decodeGpsBasic(const uint8_t* p, int len)
{
  if (len < 15) return;

  int32_t latRaw = readS32BE(p + 0);
  int32_t lonRaw = readS32BE(p + 4);
  uint16_t groundSpeedRaw = readU16BE(p + 8);
  uint16_t headingRaw = readU16BE(p + 10);
  uint16_t altitudeRaw = readU16BE(p + 12);
  uint8_t satellites = p[14];

  tel.lat = latRaw / 10000000.0;
  tel.lon = lonRaw / 10000000.0;
  tel.gpsSpeed = groundSpeedRaw / 100.0f;
  tel.gpsHeading = headingRaw / 100.0f;
  tel.gpsAltitude = (int)altitudeRaw - 1000;
  tel.sats = satellites;

  if (satellites >= 4 && fabs(tel.lat) > 0.000001 && fabs(tel.lon) > 0.000001) {
    lastGoodLat = tel.lat;
    lastGoodLon = tel.lon;
    lastGoodAlt = tel.gpsAltitude;
    lastGoodGpsMs = millis();
    lastGoodGpsSet = true;
  }
}

// =====================================================
// PARSER CRSF
// =====================================================

bool decodeCrsfFrame(const uint8_t* crsf, int crsfLenTotal)
{
  if (crsfLenTotal < 4) return false;

  uint8_t frameLen = crsf[1];

  if (frameLen < 2 || frameLen > 62) return false;

  int expectedTotal = frameLen + 2;

  if (expectedTotal > crsfLenTotal) return false;

  uint8_t type = crsf[2];

  const uint8_t* payload = crsf + 3;
  int payloadLen = frameLen - 2;

  tel.tramasCrsf++;
  tel.ultimoCrsfMs = millis();

  if (esTramaTelemetriaAvion(type)) {
    tel.ultimaTelemetriaAvionMs = millis();
    tel.tramasTelemetriaAvion++;
    tel.recibeDatos = true;
    tuvoPrimerAircraftTelemetry = true;
  }

  switch (type) {
    case 0x02:
      decodeGpsBasic(payload, payloadLen);
      break;

    case 0x07:
      decodeVariometer(payload, payloadLen);
      break;

    case 0x08:
      decodeBattery(payload, payloadLen);
      break;

    case 0x09:
      decodeBaroAltitude(payload, payloadLen);
      break;

    case 0x14:
      decodeLinkStatistics(payload, payloadLen);
      break;

    case 0x1E:
      decodeAttitude(payload, payloadLen);
      break;

    case 0x21:
      decodeFlightMode(payload, payloadLen);
      break;

    default:
      break;
  }

  return true;
}

// =====================================================
// PARSER MSPv2
// =====================================================

bool parseMspV2Frames(const uint8_t* data, int len)
{
  bool encontro = false;
  int pos = 0;

  while (pos <= len - 9) {
    if (data[pos] == '$' && data[pos + 1] == 'X') {

      uint16_t function = readU16LE(data + pos + 4);
      uint16_t payloadSize = readU16LE(data + pos + 6);

      int totalMspLen = 8 + payloadSize + 1;

      if (payloadSize > 512) {
        pos++;
        continue;
      }

      if (pos + totalMspLen > len) {
        return encontro;
      }

      const uint8_t* payload = data + pos + 8;

      tel.tramasMspV2++;
      encontro = true;

      if (function == 17) {
        decodeCrsfFrame(payload, payloadSize);
      }

      pos += totalMspLen;
    }
    else {
      pos++;
    }
  }

  return encontro;
}

// =====================================================
// PROCESADOR UDP
// =====================================================

void procesarPaqueteUdp(const uint8_t* data, int len)
{
  bool interpretado = false;

  if (parseMspV2Frames(data, len)) {
    interpretado = true;
  }

  if (!interpretado) {
    tel.paquetesDesconocidos++;
  }
}

void leerUdp()
{
  int packetSize = udp.parsePacket();

  if (packetSize > 0) {
    uint8_t buffer[768];

    int len = udp.read(buffer, sizeof(buffer));

    tel.paquetesUdp++;
    tel.ultimoUdpMs = millis();

    tuvoPrimerUdp = true;

    procesarPaqueteUdp(buffer, len);
  }

  if (tuvoPrimerAircraftTelemetry &&
      millis() - tel.ultimaTelemetriaAvionMs > TIMEOUT_AIRCRAFT_TELEM_MS) {
    tel.recibeDatos = false;
  }
}

// =====================================================
// ESTADO DE ENLACES
// =====================================================

bool udpOk()
{
  if (!tuvoPrimerUdp) return false;
  return (millis() - tel.ultimoUdpMs) <= TIMEOUT_UDP_MS;
}

bool aircraftTelemetryOk()
{
  if (!tuvoPrimerAircraftTelemetry) return false;
  return (millis() - tel.ultimaTelemetriaAvionMs) <= TIMEOUT_AIRCRAFT_TELEM_MS;
}

uint32_t udpAgeSec()
{
  if (!tuvoPrimerUdp) return 0;
  return (millis() - tel.ultimoUdpMs) / 1000;
}

uint32_t aircraftTelemetryAgeSec()
{
  if (!tuvoPrimerAircraftTelemetry) return 0;
  return (millis() - tel.ultimaTelemetriaAvionMs) / 1000;
}

// =====================================================
// HOME / NAVEGACION
// =====================================================

double degToRadD(double deg)
{
  return deg * 3.14159265358979323846 / 180.0;
}

double radToDegD(double rad)
{
  return rad * 180.0 / 3.14159265358979323846;
}

float normalize360(float deg)
{
  while (deg < 0.0f) deg += 360.0f;
  while (deg >= 360.0f) deg -= 360.0f;
  return deg;
}

float normalize180(float deg)
{
  deg = normalize360(deg);
  if (deg > 180.0f) deg -= 360.0f;
  return deg;
}

// =====================================================
// DETECCION AUTOMATICA DE CELDAS / VOLTAJE POR CELDA
// =====================================================

int detectarCeldas(float voltaje)
{
  if (voltaje < 5.0f)  return 0;
  if (voltaje <= 8.8f)  return 2;  // 2S
  if (voltaje <= 13.2f) return 3;  // 3S
  if (voltaje <= 17.6f) return 4;  // 4S
  if (voltaje <= 22.0f) return 5;  // 5S
  if (voltaje <= 26.4f) return 6;  // 6S
  if (voltaje <= 30.8f) return 7;  // 7S
  if (voltaje <= 35.2f) return 8;  // 8S
  return 0;
}

float voltajePorCelda()
{
  int celdas = detectarCeldas(tel.voltaje);

  if (celdas <= 0) {
    return 0.0f;
  }

  return tel.voltaje / celdas;
}

uint16_t colorVoltajeCelda()
{
  float vCell = voltajePorCelda();

  if (vCell >= 4.10f) return TFT_GREEN;
  if (vCell >= 3.80f) return TFT_YELLOW;
  if (vCell >= 3.60f) return TFT_ORANGE;

  return TFT_RED;
}

bool gpsValidoParaHome()
{
  if (tel.sats < 4) return false;
  if (fabs(tel.lat) < 0.000001 && fabs(tel.lon) < 0.000001) return false;
  return true;
}

bool modeloArmadoPorModo()
{
  if (tel.modoVuelo.length() == 0) return false;
  if (tel.modoVuelo == "---") return false;
  if (tel.modoVuelo == "WAIT") return false;
  return true;
}

float headingActual()
{
  if (tel.gpsHeading > 0.5f) {
    return normalize360(tel.gpsHeading);
  }
  return normalize360(tel.yaw);
}

float calcularDistanciaM(double lat1, double lon1, double lat2, double lon2)
{
  const double Rearth = 6371000.0;
  double phi1 = degToRadD(lat1);
  double phi2 = degToRadD(lat2);
  double dPhi = degToRadD(lat2 - lat1);
  double dLambda = degToRadD(lon2 - lon1);

  double a = sin(dPhi / 2.0) * sin(dPhi / 2.0) +
             cos(phi1) * cos(phi2) *
             sin(dLambda / 2.0) * sin(dLambda / 2.0);

  double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
  return (float)(Rearth * c);
}

float calcularBearingDeg(double lat1, double lon1, double lat2, double lon2)
{
  double phi1 = degToRadD(lat1);
  double phi2 = degToRadD(lat2);
  double dLambda = degToRadD(lon2 - lon1);

  double y = sin(dLambda) * cos(phi2);
  double x = cos(phi1) * sin(phi2) -
             sin(phi1) * cos(phi2) * cos(dLambda);

  double brng = radToDegD(atan2(y, x));
  return normalize360((float)brng);
}

void actualizarHome()
{
  if (!homeSet && modeloArmadoPorModo() && gpsValidoParaHome()) {
    homeLat = tel.lat;
    homeLon = tel.lon;
    homeSet = true;
    homeSetMs = millis();
  }

  if (homeSet && gpsValidoParaHome()) {
    homeDistanceM = calcularDistanciaM(tel.lat, tel.lon, homeLat, homeLon);
    homeBearingDeg = calcularBearingDeg(tel.lat, tel.lon, homeLat, homeLon);
    homeRelativeBearingDeg = normalize180(homeBearingDeg - headingActual());
  }
}

String generarPlusCode(double lat, double lon)
{
  const char* alphabet = "23456789CFGHJMPQRVWX";

  if (lat < -90.0) lat = -90.0;
  if (lat >= 90.0) lat = 90.0 - 0.000000001;

  while (lon < -180.0) lon += 360.0;
  while (lon >= 180.0) lon -= 360.0;

  long latVal = (long)floor((lat + 90.0) * 8000.0);
  long lonVal = (long)floor((lon + 180.0) * 8000.0);

  long divisors[5] = {160000, 8000, 400, 20, 1};
  String code = "";

  for (int i = 0; i < 5; i++) {
    int latDigit = (latVal / divisors[i]) % 20;
    int lonDigit = (lonVal / divisors[i]) % 20;
    code += alphabet[latDigit];
    code += alphabet[lonDigit];
    if (i == 3) code += "+";
  }

  return code;
}

String plusCodeRecuperacion()
{
  if (lastGoodGpsSet) {
    return generarPlusCode(lastGoodLat, lastGoodLon);
  }
  return generarPlusCode(tel.lat, tel.lon);
}

String generarUrlGpsRecuperacion()
{
  String url = "https://maps.google.com/?q=";
  if (lastGoodGpsSet) {
    url += String(lastGoodLat, 7);
    url += ",";
    url += String(lastGoodLon, 7);
  }
  else {
    url += String(tel.lat, 7);
    url += ",";
    url += String(tel.lon, 7);
  }
  return url;
}

void qrDisplayCallback(esp_qrcode_handle_t qrcode)
{
  int size = esp_qrcode_get_size(qrcode);
  const int quiet = 2;
  int totalModules = size + quiet * 2;
  int qrPx = totalModules * qrDrawScale;

  spr.fillRect(qrDrawX, qrDrawY, qrPx, qrPx, TFT_WHITE);

  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      if (esp_qrcode_get_module(qrcode, x, y)) {
        spr.fillRect(qrDrawX + (x + quiet) * qrDrawScale,
                     qrDrawY + (y + quiet) * qrDrawScale,
                     qrDrawScale,
                     qrDrawScale,
                     TFT_BLACK);
      }
    }
  }
  spr.drawRect(qrDrawX, qrDrawY, qrPx, qrPx, TFT_DARKGREY);
}

void drawRecoveryQr(int x0, int y0, int escala)
{
  if (!lastGoodGpsSet) {
    spr.fillRect(x0, y0, 76, 76, TFT_BLACK);
    spr.drawRect(x0, y0, 76, 76, TFT_DARKGREY);
    spr.setTextDatum(MC_DATUM);
    spr.setTextSize(1);
    spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
    spr.drawString("NO GPS", x0 + 38, y0 + 30);
    spr.drawString("QR", x0 + 38, y0 + 45);
    return;
  }

  qrDrawX = x0;
  qrDrawY = y0;
  qrDrawScale = escala;

  String qrText = generarUrlGpsRecuperacion();

  esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
  cfg.display_func = qrDisplayCallback;
  cfg.max_qrcode_version = 5;
  cfg.qrcode_ecc_level = ESP_QRCODE_ECC_LOW;

  esp_qrcode_generate(&cfg, qrText.c_str());
}

// =====================================================
// BACKLIGHT PWM / BARRA DE BRILLO
// =====================================================

void backlightPwmAttach()
{
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(PIN_BL, BL_PWM_FREQ, BL_PWM_RES);
#else
  ledcSetup(BL_PWM_CH, BL_PWM_FREQ, BL_PWM_RES);
  ledcAttachPin(PIN_BL, BL_PWM_CH);
#endif
}

void backlightPwmWrite(uint8_t value)
{
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(PIN_BL, value);
#else
  ledcWrite(BL_PWM_CH, value);
#endif
}

void cargarBrillo()
{
  prefs.begin("gs", false);
  brightnessIndex = prefs.getInt("bright", 3);

  if (brightnessIndex < 0 || brightnessIndex >= BRIGHTNESS_COUNT) {
    brightnessIndex = 3;
  }
}

void guardarBrillo()
{
  prefs.putInt("bright", brightnessIndex);
}

int brilloPorcentaje()
{
  switch (brightnessIndex) {
    case 0: return 25;
    case 1: return 50;
    case 2: return 75;
    default: return 100;
  }
}

void aplicarBrillo()
{
  backlightPwmWrite(BRIGHTNESS_LEVELS[brightnessIndex]);
}

bool mostrarOverlayBrillo()
{
  return millis() < brightnessOverlayUntilMs;
}

void abrirModoBrillo()
{
  brightnessOverlayUntilMs = millis() + 5000;
}

void cambiarBrillo(int delta)
{
  brightnessIndex += delta;

  if (brightnessIndex < 0) brightnessIndex = 0;
  if (brightnessIndex >= BRIGHTNESS_COUNT) brightnessIndex = BRIGHTNESS_COUNT - 1;

  aplicarBrillo();
  guardarBrillo();
  brightnessOverlayUntilMs = millis() + 5000;
}

void drawBrightnessOverlay()
{
  spr.fillSprite(TFT_BLACK);
  spr.setTextDatum(MC_DATUM);

  spr.setTextSize(2);
  spr.setTextColor(TFT_CYAN, TFT_BLACK);
  spr.drawString("BRIGHTNESS", SCREEN_W / 2, 35);

  int pct = brilloPorcentaje();
  char buf[20];
  sprintf(buf, "%d%%", pct);

  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString(buf, SCREEN_W / 2, 70);

  int barX = 50;
  int barY = 105;
  int barW = 220;
  int barH = 18;

  spr.drawRect(barX, barY, barW, barH, TFT_WHITE);
  spr.fillRect(barX + 2, barY + 2, barW - 4, barH - 4, TFT_DARKGREY);

  int fillW = map(pct, 0, 100, 0, barW - 4);
  spr.fillRect(barX + 2, barY + 2, fillW, barH - 4, TFT_GREEN);

  spr.setTextSize(1);
  spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
  spr.drawString("PREV -   NEXT +", SCREEN_W / 2, 145);
  spr.drawString("Long NEXT opens brightness", SCREEN_W / 2, 158);

  spr.pushSprite(0, 0);
}

// =====================================================
// BOTONES
// =====================================================

void leerBotones()
{
  static bool nextWasDown = false;
  static uint32_t nextDownMs = 0;
  static bool nextLongHandled = false;
  static bool prevWasDown = false;

  const uint32_t LONG_PRESS_MS = 1500;
  const uint32_t DEBOUNCE_CONFIRM_MS = 25;

  bool nextNow = (digitalRead(BTN_NEXT) == LOW);
  bool prevNow = (digitalRead(BTN_PREV) == LOW);
  bool modoBrilloActivo = mostrarOverlayBrillo();

  if (nextNow && !nextWasDown) {
    delay(DEBOUNCE_CONFIRM_MS);
    if (digitalRead(BTN_NEXT) == LOW) {
      nextWasDown = true;
      nextDownMs = millis();
      nextLongHandled = false;
    }
  }

  if (nextWasDown && nextNow && !nextLongHandled) {
    if (millis() - nextDownMs >= LONG_PRESS_MS) {
      abrirModoBrillo();
      nextLongHandled = true;
    }
  }

  if (nextWasDown && !nextNow) {
    if (!nextLongHandled) {
      if (modoBrilloActivo) {
        cambiarBrillo(+1);
      }
      else {
        pantallaActual++;
        if (pantallaActual >= TOTAL_PANTALLAS) pantallaActual = 0;
      }
    }
    nextWasDown = false;
  }

  if (prevNow && !prevWasDown) {
    delay(DEBOUNCE_CONFIRM_MS);
    if (digitalRead(BTN_PREV) == LOW) {
      prevWasDown = true;
    }
  }

  if (prevWasDown && !prevNow) {
    if (modoBrilloActivo) {
      cambiarBrillo(-1);
    }
    else {
      pantallaActual--;
      if (pantallaActual < 0) pantallaActual = TOTAL_PANTALLAS - 1;
    }
    prevWasDown = false;
  }
}

// =====================================================
// UTILIDADES GRAFICAS
// =====================================================

float degToRad(float deg)
{
  return deg * 3.14159265f / 180.0f;
}

void rotatePoint(float x, float y, float rollRad, int &rx, int &ry)
{
  float xr = x * cos(rollRad) - y * sin(rollRad);
  float yr = x * sin(rollRad) + y * cos(rollRad);

  rx = CX + (int)xr;
  ry = CY + (int)yr;
}

String nombrePantalla()
{
  switch (pantallaActual) {
    case P_GENERAL: return "GENERAL";
    case P_HORIZONTE: return "HORIZONTE";
    case P_ENERGIA: return "ENERGIA";
    case P_GPS: return "GPS";
    case P_DEBUG: return "DEBUG";
    default: return "---";
  }
}

void drawTopStatusBar(const char* titulo)
{
  char buf[64];

  spr.fillRect(0, 0, SCREEN_W, 16, TFT_BLACK);
  spr.setTextDatum(TL_DATUM);
  spr.setTextSize(1);

  if (!udpOk()) {
    spr.setTextColor(TFT_RED, TFT_BLACK);
    spr.drawString("UDP LOST", 4, 3);
  }
  else if (!aircraftTelemetryOk()) {
    spr.setTextColor(TFT_ORANGE, TFT_BLACK);
    spr.drawString("TEL LOST", 4, 3);
  }
  else {
    spr.setTextColor(TFT_GREEN, TFT_BLACK);
    spr.drawString("LIVE", 4, 3);
  }

  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString(titulo, 80, 3);

  sprintf(buf, "%d/%d", pantallaActual + 1, TOTAL_PANTALLAS);
  spr.drawString(buf, 286, 3);

  spr.drawFastHLine(0, 16, SCREEN_W, TFT_DARKGREY);
}

// =====================================================
// PANTALLAS DE SISTEMA
// =====================================================

void drawSystemScreen(const char* titulo, const char* linea1, const char* linea2, uint16_t colorTitulo)
{
  spr.fillSprite(TFT_BLACK);

  spr.setTextDatum(MC_DATUM);

  spr.setTextColor(colorTitulo, TFT_BLACK);
  spr.setTextSize(2);
  spr.drawString(titulo, SCREEN_W / 2, 28);

  spr.drawFastHLine(20, 50, SCREEN_W - 40, TFT_DARKGREY);

  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextSize(2);
  spr.drawString(linea1, SCREEN_W / 2, 88);

  spr.setTextSize(1);
  spr.setTextColor(TFT_CYAN, TFT_BLACK);
  spr.drawString(linea2, SCREEN_W / 2, 118);

  char buf[64];

  sprintf(buf, "UDP:%lu  MSP:%lu  CRSF:%lu",
          tel.paquetesUdp,
          tel.tramasMspV2,
          tel.tramasCrsf);

  spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
  spr.drawString(buf, SCREEN_W / 2, 150);

  spr.pushSprite(0, 0);
}

void drawBootScreen()
{
  drawSystemScreen("GROUND STATION", "BOOTING...", "ESP32 + TFT + BACKPACK", TFT_CYAN);
}

void drawConnectingWifiScreen()
{
  static uint8_t dots = 0;
  dots = (dots + 1) % 4;

  String msg = "CONNECTING WIFI";

  for (uint8_t i = 0; i < dots; i++) {
    msg += ".";
  }

  drawSystemScreen("GROUND STATION", msg.c_str(), "ExpressLRS Backpack", TFT_YELLOW);
}

void drawWaitingUdpScreen()
{
  static uint8_t dots = 0;
  dots = (dots + 1) % 4;

  String msg = "WAITING UDP";

  for (uint8_t i = 0; i < dots; i++) {
    msg += ".";
  }

  drawSystemScreen("GROUND STATION", msg.c_str(), "WiFi OK, waiting Backpack packets", TFT_GREEN);
}

void drawWaitingAircraftTelemetryScreen()
{
  static uint8_t dots = 0;
  dots = (dots + 1) % 4;

  String msg = "WAITING AIRCRAFT";

  for (uint8_t i = 0; i < dots; i++) {
    msg += ".";
  }

  drawSystemScreen("GROUND STATION", msg.c_str(), "UDP OK, waiting aircraft telemetry", TFT_ORANGE);
}

// =====================================================
// HORIZONTE ARTIFICIAL
// =====================================================

void drawHorizonBackground(float pitch, float roll)
{
  float rollRad = degToRad(roll);
  float tanRoll = tan(rollRad);

  float pitchOffset = pitch * PITCH_PIXELS_PER_DEG;

  for (int y = CY - R; y <= CY + R; y++) {
    int dy = y - CY;
    int span = sqrt((R * R) - (dy * dy));

    int xStart = CX - span;
    int xEnd   = CX + span;

    for (int x = xStart; x <= xEnd; x++) {
      int dx = x - CX;

      float horizonY = CY + pitchOffset + tanRoll * dx;

      if (y < horizonY) {
        spr.drawPixel(x, y, SKY_COLOR);
      }
      else {
        spr.drawPixel(x, y, GROUND_COLOR);
      }
    }
  }

  int x1 = CX - R;
  int x2 = CX + R;

  int y1 = CY + pitchOffset + tanRoll * (x1 - CX);
  int y2 = CY + pitchOffset + tanRoll * (x2 - CX);

  spr.drawLine(x1, y1, x2, y2, WHITE_COLOR);
  spr.drawLine(x1, y1 + 1, x2, y2 + 1, WHITE_COLOR);
}

void drawPitchLadder(float pitch, float roll)
{
  float rollRad = degToRad(roll);
  float pitchOffset = pitch * PITCH_PIXELS_PER_DEG;

  spr.setTextDatum(MC_DATUM);
  spr.setTextSize(1);
  spr.setTextColor(WHITE_COLOR, TFT_BLACK);

  for (int p = -30; p <= 30; p += 10) {
    if (p == 0) continue;

    float yBase = pitchOffset - (p * PITCH_PIXELS_PER_DEG);

    int x1, y1, x2, y2;
    int x3, y3, x4, y4;

    rotatePoint(-36, yBase, rollRad, x1, y1);
    rotatePoint(-12, yBase, rollRad, x2, y2);

    rotatePoint(12, yBase, rollRad, x3, y3);
    rotatePoint(36, yBase, rollRad, x4, y4);

    spr.drawLine(x1, y1, x2, y2, WHITE_COLOR);
    spr.drawLine(x3, y3, x4, y4, WHITE_COLOR);

    int txL, tyL, txR, tyR;

    rotatePoint(-50, yBase, rollRad, txL, tyL);
    rotatePoint(50, yBase, rollRad, txR, tyR);

    char buf[8];
    sprintf(buf, "%d", abs(p));

    spr.drawString(buf, txL, tyL);
    spr.drawString(buf, txR, tyR);
  }
}

void drawRollScale()
{
  spr.drawCircle(CX, CY, R, WHITE_COLOR);
  spr.drawCircle(CX, CY, R + 1, TFT_DARKGREY);

  for (int a = -60; a <= 60; a += 15) {
    float ang = degToRad(a - 90);

    int r1;
    int r2;

    if (a % 30 == 0) {
      r1 = R - 11;
      r2 = R - 1;
    }
    else {
      r1 = R - 7;
      r2 = R - 1;
    }

    int x1 = CX + cos(ang) * r1;
    int y1 = CY + sin(ang) * r1;

    int x2 = CX + cos(ang) * r2;
    int y2 = CY + sin(ang) * r2;

    spr.drawLine(x1, y1, x2, y2, WHITE_COLOR);
  }

  spr.fillTriangle(CX, CY - R + 4,
                   CX - 7, CY - R + 16,
                   CX + 7, CY - R + 16,
                   YELLOW_COLOR);
}

void drawAircraftReference()
{
  spr.fillCircle(CX, CY, 4, YELLOW_COLOR);

  spr.drawLine(CX - 48, CY, CX - 16, CY, YELLOW_COLOR);
  spr.drawLine(CX + 16, CY, CX + 48, CY, YELLOW_COLOR);

  spr.drawLine(CX - 48, CY, CX - 48, CY + 7, YELLOW_COLOR);
  spr.drawLine(CX + 48, CY, CX + 48, CY + 7, YELLOW_COLOR);

  spr.drawLine(CX, CY, CX, CY - 13, YELLOW_COLOR);

  spr.fillCircle(CX, CY, 2, TFT_BLACK);
}

void drawHomeArrow(int x, int y, int len)
{
  if (!homeSet) {
    spr.setTextDatum(MC_DATUM);
    spr.setTextSize(1);
    spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
    spr.drawString("NO HOME", x, y);
    return;
  }

  float rad = degToRad(homeRelativeBearingDeg);

  int tipX = x + sin(rad) * len;
  int tipY = y - cos(rad) * len;

  spr.drawLine(x, y, tipX, tipY, TFT_MAGENTA);
  spr.drawLine(x + 1, y, tipX + 1, tipY, TFT_MAGENTA);

  float leftRad = rad + degToRad(150);
  float rightRad = rad - degToRad(150);

  int lX = tipX + sin(leftRad) * 9;
  int lY = tipY - cos(leftRad) * 9;
  int rX = tipX + sin(rightRad) * 9;
  int rY = tipY - cos(rightRad) * 9;

  spr.fillTriangle(tipX, tipY, lX, lY, rX, rY, TFT_MAGENTA);

  spr.setTextDatum(MC_DATUM);
  spr.setTextSize(1);
  spr.setTextColor(TFT_MAGENTA, TFT_BLACK);
  spr.drawString("HOME", x, y + 12);
}

void drawMiniCompass(int x, int y)
{
  spr.drawCircle(x, y, 17, TFT_WHITE);
  spr.setTextDatum(MC_DATUM);
  spr.setTextSize(1);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);

  spr.drawString("N", x, y - 13);
  spr.drawString("S", x, y + 13);
  spr.drawString("E", x + 13, y);
  spr.drawString("W", x - 13, y);

  float heading = headingActual();
  float rad = degToRad(heading);

  int nx = x + sin(rad) * 10;
  int ny = y - cos(rad) * 10;

  spr.drawLine(x, y, nx, ny, TFT_YELLOW);
  spr.fillCircle(nx, ny, 2, TFT_YELLOW);
}

// =====================================================
// PANTALLA 0 - GENERAL
// =====================================================

void drawScreenGeneral()
{
  char buf[64];

  spr.fillSprite(TFT_BLACK);
  drawTopStatusBar("GENERAL");

  spr.setTextDatum(TL_DATUM);
  spr.setTextSize(2);

  spr.setTextColor(TFT_YELLOW, TFT_BLACK);
  spr.drawString("MODE", 8, 28);

  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString(tel.modoVuelo, 95, 28);

  spr.setTextSize(2);
  spr.setTextColor(TFT_GREEN, TFT_BLACK);
  spr.drawString("BAT", 8, 58);

  sprintf(buf, "%.1fV", tel.voltaje);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString(buf, 75, 58);

  if (tel.bateriaRestante >= 0) {
    sprintf(buf, "%d%%", tel.bateriaRestante);
  }
  else {
    sprintf(buf, "---");
  }
  spr.drawString(buf, 180, 58);

  spr.setTextColor(TFT_CYAN, TFT_BLACK);
  spr.drawString("LQ", 8, 88);

  sprintf(buf, "%d/%d%%", tel.uplinkLQ, tel.downlinkLQ);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString(buf, 75, 88);

  spr.setTextColor(TFT_CYAN, TFT_BLACK);
  spr.drawString("SNR", 8, 118);

  sprintf(buf, "%d/%ddB", tel.uplinkSNR, tel.downlinkSNR);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString(buf, 75, 118);

  spr.setTextColor(TFT_MAGENTA, TFT_BLACK);
  spr.drawString("SAT", 205, 88);

  sprintf(buf, "%d", tel.sats);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString(buf, 265, 88);

  spr.pushSprite(0, 0);
}

// =====================================================
// PANTALLA 1 - HORIZONTE
// =====================================================

void drawScreenHorizonte()
{
  spr.fillSprite(TFT_BLACK);

  drawHorizonBackground(tel.pitch, tel.roll);
  drawPitchLadder(tel.pitch, tel.roll);
  drawRollScale();
  drawAircraftReference();

  drawTopStatusBar("HORIZONTE");

  char buf[48];

  spr.setTextDatum(TL_DATUM);
  spr.setTextSize(1);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);

  sprintf(buf, "ALT %dm", tel.gpsAltitude);
  spr.drawString(buf, 6, 24);

  sprintf(buf, "PIT %+04.1f", tel.pitch);
  spr.drawString(buf, 6, 38);

  int heading = (int)headingActual();
  sprintf(buf, "SPD %.1f", tel.gpsSpeed);
  spr.drawString(buf, 238, 24);

  sprintf(buf, "HDG %03d", heading);
  spr.drawString(buf, 238, 38);

  sprintf(buf, "BAT %.1fV", tel.voltaje);
  spr.drawString(buf, 238, 52);

  // Flecha HOME tipo OSD
  drawHomeArrow(160, 122, 25);

  // Distancia HOME abajo izquierda
  spr.setTextDatum(TL_DATUM);
  spr.setTextSize(1);
  if (homeSet) {
    spr.setTextColor(TFT_MAGENTA, TFT_BLACK);
    sprintf(buf, "HOME %.0fm", homeDistanceM);
    spr.drawString(buf, 6, 150);
  }
  else {
    spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
    spr.drawString("HOME ---", 6, 150);
  }

  // Mini brujula abajo derecha
  drawMiniCompass(286, 150);

  spr.pushSprite(0, 0);
}

// =====================================================
// PANTALLA 2 - ENERGIA
// =====================================================

void drawScreenEnergia()
{
  char buf[64];

  spr.fillSprite(TFT_BLACK);
  drawTopStatusBar("ENERGIA");

  spr.setTextDatum(TL_DATUM);

  spr.setTextSize(2);
  spr.setTextColor(TFT_GREEN, TFT_BLACK);
  spr.drawString("POWER", 8, 25);

  spr.setTextColor(TFT_WHITE, TFT_BLACK);

  sprintf(buf, "Voltaje : %.1f V", tel.voltaje);
  spr.drawString(buf, 8, 48);

  int celdas = detectarCeldas(tel.voltaje);
  float vCell = voltajePorCelda();
  uint16_t vCellColor = colorVoltajeCelda();

  if (celdas > 0) {
    spr.setTextColor(vCellColor, TFT_BLACK);
    sprintf(buf, "Bateria : %dS", celdas);
    spr.drawString(buf, 8, 73);

    sprintf(buf, "V/Celda : %.2f V", vCell);
    spr.drawString(buf, 8, 98);
  }
  else {
    spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
    spr.drawString("Bateria : ?", 8, 73);
    spr.drawString("V/Celda : --.-- V", 8, 98);
  }

  spr.setTextColor(TFT_WHITE, TFT_BLACK);

  sprintf(buf, "Corr.   : %.1f A", tel.corriente);
  spr.drawString(buf, 8, 123);

  spr.setTextSize(1);
  sprintf(buf, "Consumo: %lu mAh", tel.consumoMah);
  spr.drawString(buf, 8, 150);

  if (tel.bateriaRestante >= 0) {
    sprintf(buf, "INAV %d%%", tel.bateriaRestante);
  }
  else {
    sprintf(buf, "INAV ---");
  }
  spr.drawString(buf, 130, 150);

  spr.setTextColor(TFT_CYAN, TFT_BLACK);
  spr.drawString(("RF " + tel.rfPowerTexto), 210, 150);

  spr.pushSprite(0, 0);
}

// =====================================================
// PANTALLA 3 - GPS estilo Garmin
// Coordenadas arriba, datos en 2 columnas abajo
// =====================================================

void drawScreenGps()
{
  char buf[96];
  bool emergencia = !udpOk() || !aircraftTelemetryOk();

  spr.fillSprite(TFT_BLACK);
  drawTopStatusBar("GPS");
  spr.setTextDatum(TL_DATUM);

  if (emergencia) {
    spr.setTextSize(2);
    spr.setTextColor(TFT_RED, TFT_BLACK);
    spr.drawString("RECOVERY", 8, 22);

    spr.setTextSize(1);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);

    if (lastGoodGpsSet) {
      sprintf(buf, "LAT %.7f", lastGoodLat);
      spr.drawString(buf, 8, 46);

      sprintf(buf, "LON %.7f", lastGoodLon);
      spr.drawString(buf, 8, 60);

      sprintf(buf, "ALT %dm", lastGoodAlt);
      spr.drawString(buf, 8, 74);

      spr.setTextColor(TFT_YELLOW, TFT_BLACK);
      spr.drawString("PLUS", 8, 91);
      spr.setTextColor(TFT_WHITE, TFT_BLACK);
      spr.drawString(plusCodeRecuperacion(), 8, 104);
    }
    else {
      spr.drawString("No last GPS position", 8, 48);
      spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
      spr.drawString("PLUS ---", 8, 70);
    }

    drawRecoveryQr(214, 30, 2);

    if (homeSet && lastGoodGpsSet) {
      float distHomeLast = calcularDistanciaM(lastGoodLat, lastGoodLon, homeLat, homeLon);
      spr.setTextColor(TFT_MAGENTA, TFT_BLACK);
      sprintf(buf, "HOME %.0fm", distHomeLast);
      spr.drawString(buf, 8, 124);
    }

    spr.setTextColor(TFT_CYAN, TFT_BLACK);
    spr.drawString("Scan QR -> Google Maps", 8, 150);
  }
  else {
    spr.setTextSize(2);
    spr.setTextColor(TFT_MAGENTA, TFT_BLACK);
    spr.drawString("GPS", 8, 22);

    spr.setTextColor(TFT_WHITE, TFT_BLACK);

    sprintf(buf, "LAT %.7f", tel.lat);
    spr.drawString(buf, 8, 46);

    sprintf(buf, "LON %.7f", tel.lon);
    spr.drawString(buf, 8, 70);

    spr.drawFastHLine(8, 98, 304, TFT_DARKGREY);

    spr.setTextSize(2);

    spr.setTextColor(TFT_CYAN, TFT_BLACK);
    spr.drawString("SAT", 10, 106);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    sprintf(buf, "%d", tel.sats);
    spr.drawString(buf, 65, 106);

    spr.setTextColor(TFT_CYAN, TFT_BLACK);
    spr.drawString("ALT", 10, 132);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    sprintf(buf, "%dm", tel.gpsAltitude);
    spr.drawString(buf, 65, 132);

    spr.setTextColor(TFT_CYAN, TFT_BLACK);
    spr.drawString("SPD", 150, 106);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    sprintf(buf, "%.1f", tel.gpsSpeed);
    spr.drawString(buf, 205, 106);

    spr.setTextColor(TFT_CYAN, TFT_BLACK);
    spr.drawString("HDG", 150, 132);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    sprintf(buf, "%.0f", tel.gpsHeading);
    spr.drawString(buf, 205, 132);

    spr.setTextSize(1);
    if (homeSet) {
      spr.setTextColor(TFT_MAGENTA, TFT_BLACK);
      sprintf(buf, "HOME %.0fm  BRG %.0f  REL %.0f", homeDistanceM, homeBearingDeg, homeRelativeBearingDeg);
      spr.drawString(buf, 8, 160);
    }
    else {
      spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
      spr.drawString("HOME not set - waiting mode != WAIT", 8, 160);
    }
  }

  spr.pushSprite(0, 0);
}

// =====================================================
// PANTALLA 4 - DEBUG
// 2 columnas
// =====================================================

void drawScreenDebug()
{
  char buf[80];

  spr.fillSprite(TFT_BLACK);
  drawTopStatusBar("DEBUG");

  spr.setTextDatum(TL_DATUM);
  spr.setTextSize(1);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);

  // Columna izquierda
  sprintf(buf, "UDP    %lu", tel.paquetesUdp);
  spr.drawString(buf, 8, 28);

  sprintf(buf, "MSPv2  %lu", tel.tramasMspV2);
  spr.drawString(buf, 8, 45);

  sprintf(buf, "CRSF   %lu", tel.tramasCrsf);
  spr.drawString(buf, 8, 62);

  sprintf(buf, "AIR    %lu", tel.tramasTelemetriaAvion);
  spr.drawString(buf, 8, 79);

  sprintf(buf, "UNK    %lu", tel.paquetesDesconocidos);
  spr.drawString(buf, 8, 96);

  // Columna derecha
  sprintf(buf, "UDP AGE %lu s", udpAgeSec());
  spr.drawString(buf, 170, 28);

  sprintf(buf, "TEL AGE %lu s", aircraftTelemetryAgeSec());
  spr.drawString(buf, 170, 45);

  sprintf(buf, "UP LQ   %d", tel.uplinkLQ);
  spr.drawString(buf, 170, 62);

  sprintf(buf, "DN LQ   %d", tel.downlinkLQ);
  spr.drawString(buf, 170, 79);

  sprintf(buf, "HOME %s", homeSet ? "SET" : "---");
  spr.drawString(buf, 170, 96);

  sprintf(buf, "GPS %s", lastGoodGpsSet ? "LAST OK" : "---");
  spr.drawString(buf, 170, 113);

  sprintf(buf, "RF %s", tel.rfPowerTexto.c_str());
  spr.drawString(buf, 170, 130);

  // IP abajo en ancho completo
  sprintf(buf, "IP %s", WiFi.localIP().toString().c_str());
  spr.drawString(buf, 8, 145);

  spr.pushSprite(0, 0);
}

// =====================================================
// DIBUJO SEGUN PANTALLA
// =====================================================

void drawPantallaActual()
{
  switch (pantallaActual) {
    case P_GENERAL:
      drawScreenGeneral();
      break;

    case P_HORIZONTE:
      drawScreenHorizonte();
      break;

    case P_ENERGIA:
      drawScreenEnergia();
      break;

    case P_GPS:
      drawScreenGps();
      break;

    case P_DEBUG:
      drawScreenDebug();
      break;

    default:
      drawScreenGeneral();
      break;
  }
}

bool buscarBackpackWifi()
{
  selectedBackpackSsid = "";

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(250);

  int n = WiFi.scanNetworks();

  for (int i = 0; i < n; i++) {
    String s = WiFi.SSID(i);
    if (s.startsWith(BACKPACK_PREFIX)) {
      selectedBackpackSsid = s;
      return true;
    }
  }

  return false;
}

void esperarYConectarBackpack()
{
  uint32_t lastScanDraw = 0;
  uint32_t lastWifiDraw = 0;

  while (selectedBackpackSsid.length() == 0) {
    if (millis() - lastScanDraw > 500) {
      lastScanDraw = millis();
      drawSystemScreen("GROUND STATION", "SEARCH BACKPACK", "ExpressLRS TX Backpack*", TFT_YELLOW);
    }

    Serial.println("Buscando red ExpressLRS TX Backpack...");

    if (!buscarBackpackWifi()) {
      delay(700);
    }
  }

  Serial.print("Backpack encontrado: ");
  Serial.println(selectedBackpackSsid);

  WiFi.begin(selectedBackpackSsid.c_str(), password);

  Serial.print("Conectando a Backpack");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");

    if (millis() - lastWifiDraw > 400) {
      lastWifiDraw = millis();
      drawConnectingWifiScreen();
    }
  }
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
  inicializarTelemetria();

  Serial.begin(115200);
  delay(500);

  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_NEXT, INPUT_PULLUP);

  backlightPwmAttach();
  cargarBrillo();
  aplicarBrillo();

  tft.init();
  tft.setRotation(1);

  SKY_COLOR      = tft.color565(0, 155, 255);
  GROUND_COLOR   = tft.color565(20, 145, 35);
  WHITE_COLOR    = TFT_WHITE;
  YELLOW_COLOR   = TFT_YELLOW;
  CYAN_COLOR     = TFT_CYAN;
  ORANGE_COLOR   = TFT_ORANGE;
  RED_COLOR      = TFT_RED;
  GREEN_COLOR    = TFT_GREEN;
  DARKGREY_COLOR = TFT_DARKGREY;

  spr.setColorDepth(16);
  spr.createSprite(SPR_W, SPR_H);

  tft.fillScreen(TFT_BLACK);

  drawBootScreen();
  delay(700);

  Serial.println();
  Serial.println("=================================");
  Serial.println(" GROUND STATION V2.5.1 BRIGHTNESS FIX");
  Serial.println(" BACKPACK WIFI + CRSF + TFT");
  Serial.println("=================================");

  WiFi.mode(WIFI_STA);

  esperarYConectarBackpack();

  tel.wifiConectado = true;

  Serial.println();
  Serial.println("WiFi conectado");

  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());

  if (udp.begin(UDP_PORT_LOCAL)) {
    Serial.print("Escuchando UDP en puerto ");
    Serial.println(UDP_PORT_LOCAL);
  }
  else {
    Serial.println("ERROR abriendo UDP");
  }

  drawWaitingUdpScreen();
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
  leerUdp();
  actualizarHome();
  leerBotones();

  static uint32_t lastDraw = 0;

  if (millis() - lastDraw > 80) {
    lastDraw = millis();

    if (mostrarOverlayBrillo()) {
      drawBrightnessOverlay();
    }
    else if (!tuvoPrimerUdp) {
      drawWaitingUdpScreen();
    }
    else if (!tuvoPrimerAircraftTelemetry) {
      drawWaitingAircraftTelemetryScreen();
    }
    else {
      drawPantallaActual();
    }
  }
}
