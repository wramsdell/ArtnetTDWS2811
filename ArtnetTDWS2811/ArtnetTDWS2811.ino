#include "TDWS2811.h"
#include "QNEthernet.h"
using namespace qindesign::network;

#define ARTNET_PORT 6454
#define UPDATE_DELAY 1000
#define DHCP_TIMEOUT 10000

EthernetUDP udp(64);
uint8_t buf[Ethernet.mtu() - 20 - 8];

TDWS2811 td;
uint8_t dmxData[32][1024];
uint32_t *ledBuffer;
IPAddress ip(192, 168, 1, 69);
IPAddress myDns(192, 168, 1, 1);
byte broadcast[] = {192, 168, 1, 255};
byte mac[6] = {0};
char hostname[]="Artnet0000";
uint8_t packetBuffer[1460];
unsigned long lastPacketArrivalTime = 0;
bool updateNeeded = false;

typedef struct
{
  char description[8];
  uint16_t opcode;
  uint16_t protocolVersion;
  uint8_t sequence;
  uint8_t physical;
  uint16_t universe;
  uint8_t lengthMsb;
  uint8_t lengthLsb;
} artnetHeader_t;

typedef struct
{
  artnetHeader_t header;
  uint8_t data[1460-sizeof(artnetHeader_t)];
} artnetPacket_t;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(13, OUTPUT);

  Serial.println("Start Setup");
  digitalWrite(5, HIGH);  //Deassert the shift registers' clear line
  digitalWrite(13, HIGH);

  startEthernet();
  startArtnet();

  Serial.printf("Setup complete!\n");
}

void loop() {
  appService();
  artnetService();
}

void transposeUniverse(uint8_t universe, uint8_t* data, uint16_t size)
{
  uint16_t i;
  uint8_t j;
  ledBuffer = td.getInactiveBuffer();
  uint8_t channel = universe / 2;
  uint16_t start = 512 * (universe % 2);
  for (i = 0; i < size; i++) //Byte counter
  {
    for (j = 0; j < 8; j++) //Bit counter
    {
      if ((data[i] & (1 << (7 - j))) != 0) ledBuffer[((i+start) * 8) + j] |= (1 << channel);
      else ledBuffer[((i+start) * 8) + j] &= ~(1 << channel);
    }
  }  
}

void startEthernet(void)
{
  uint8_t mac[6];
  Ethernet.macAddress(mac);  // This is informative; it retrieves, not sets
  Serial.printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  sprintf(hostname,"Artnet%02X%02X",mac[4],mac[5]);
  Ethernet.setHostname(hostname);

  // Initialize Ethernet, in this case with DHCP
  Serial.println("Starting Ethernet with DHCP...");
  if (!Ethernet.begin()) {
    Serial.println("Failed to start Ethernet");
    return;
  }
  if (!Ethernet.waitForLocalIP(DHCP_TIMEOUT)) {
    Serial.println("Failed to get IP address from DHCP");
    return;
  }

  Serial.printf("    Hostname     = ");
  Serial.print(Ethernet.hostname());
  Serial.printf("\n");
  IPAddress ip = Ethernet.localIP();
  Serial.printf("    Local IP     = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.subnetMask();
  Serial.printf("    Subnet mask  = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.broadcastIP();
  Serial.printf("    Broadcast IP = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.gatewayIP();
  Serial.printf("    Gateway      = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.dnsServerIP();
  Serial.printf("    DNS          = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);

  MDNS.begin(hostname);
//  MDNS.addService("http", "tcp", 80);
}

void startArtnet(void)
{
  udp.begin(ARTNET_PORT);
}

void artnetService(void)
{
  int packetSize = udp.parsePacket();
  if (packetSize >= 18)
  {
    udp.read(buf,packetSize);
    digitalWrite(0, HIGH);
    artnetPacket_t *receivedPacket;
    uint16_t size;
    // read the packet into packetBuffer
    receivedPacket = (artnetPacket_t *)udp.data();
    if (0 == strcmp("Art-Net",receivedPacket->header.description))
    {
      uint16_t size = (receivedPacket->header.lengthMsb << 8) + receivedPacket->header.lengthLsb;
      digitalWrite(1, HIGH);
      transposeUniverse(receivedPacket->header.universe, receivedPacket->data, size);
      digitalWrite(1, LOW);
      updateNeeded = true;
      lastPacketArrivalTime = micros();
    }
    digitalWrite(0, LOW);
  }
}

void appService(void)
{
  if (updateNeeded)
  {
    unsigned long t = micros();
    if (t > lastPacketArrivalTime + UPDATE_DELAY)
    {
      updateNeeded = false;
      td.flipBuffers();
    }
    else if (micros() < lastPacketArrivalTime)
    {
      unsigned long residue = 0 - lastPacketArrivalTime;
      if (t > (residue + UPDATE_DELAY))
      {
        updateNeeded = false;
        td.flipBuffers();

      }
    }
  }
}
