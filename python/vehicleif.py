#                                       ++++++++++++++++++
#                                  +++++++++++++++++++++++++++++
#                             +++++++                      +++++++++
#                          +++++++                               +++++++++++++
#         ++++++++++++++++++++                                         ++++++++++
#    +++++++++++++++++++++                                                     +++
#   +++++                                                                       +++
#  +++         ######### ######### ########  #########  #########   +++++++      ++
#  +++  +++++ ####  #### ######## ####  #### ##### #### #### ####  +++  ++++    +++
#  +++   ++++ ###     ## ###      ###    ### ###    ### ###    ### ++++++++   +++
#   ++++ ++++ ########## ###      ########## ###    ### ###    ### ++++    +++++
#    +++++++   ###### ## ###       ########  ###     ## ##     ###  ++++++++++
#
# Vehicle Interface Module
# ========================
#
# This module implements the interface between the LLB and HLB.
#
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

import serial, time, binascii
from enum import Enum

class ProtocolException(BaseException):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

# -----------------------------------------------------------------------------
class Vehicle:
    """Interface to the vehicle LLB over a serial port.

    Generally activated via a select call, this object maintains the interface
    to the vehicle below it and contains the current status of that vehicle.
    """

    STX_H=0x7E
    STX_L=0x8E
    VERSION=0x01
    SENSOR_TYPE_US_RANGE=10
    SENSOR_TYPE_CAR_STATUS=11
    SENSOR_TYPE_CAR_DRIVE_VECTOR=12
    SENSOR_TYPE_RANGE_SCANNER=13
    SENSOR_TYPE_WHEEL_ENCODER=14
    SENSOR_TYPE_TURRET_DATA=15
    SENSOR_TYPE_9D_SENSOR=16
    SENSOR_TYPE_BATTERY_STATUS=17
    SENSOR_TYPE_REV_TICKS=18

    CMD_TYPE_SETSPEED=24
    CMD_TYPE_SETSERVO=25
    CMD_TYPE_SETLED=26
    CMD_TYPE_CMDLED=27

    CMD_TYPE_MGMT=31

    # Specific LED commands
    CMD_LED_NONE=0
    CMD_LED_PRINT=1
    CMD_LED_CLEAR=2
    CMD_LED_MAX=2

    # Specific Management commands
    MGMT_CMD_FLASHUPDATE=0

    SIGNATURE=bytearray([0x53,0xce,0x1b,0x11,0xdd,0x8f,0x47,0x94,0xbf,0x1c,0xb0,0x62,0x64,0x71,0x4e,0xa1])
    HEADER=bytearray([0x7e,0x8e,VERSION]) # Magic number followed by version

    # ======================================
    # PARSING COMPLETE PACKETS
    # ======================================
    def _parsePacket(self):
        # Now pull out the header information
        sensorType=self.packet[0]
        sensorID=self.packet[1]
        timestamp=int.from_bytes(self.packet[2:6],byteorder='big')
        if (timestamp>self.vehicleState["timestamp"]):
            self.vehicleState["timestamp"]=timestamp
            
        # ... and trim off the header
        self.packet=self.packet[6:]

        if (sensorType==Vehicle.SENSOR_TYPE_US_RANGE):
            if (sensorID==0):
                self.vehicleState["FrontUltrasonic"]={"dist":int.from_bytes(self.packet[0:4],byteorder='big'), "ts":timestamp}
                self.updated.add("FrontUltrasonic")
                return
            if (sensorID==1):
                self.vehicleState["RearUltrasonic"]={"dist":int.from_bytes(self.packet[0:4],byteorder='big'), "ts":timestamp}
                self.updated.add("RearUltrasonic")
                return
            raise(ProtocolException("Unknown US Sensor Number"))
                
        if (sensorType==Vehicle.SENSOR_TYPE_CAR_STATUS):
            self.vehicleState["EStop"]={"front":(self.packet[0]!=0),"rear":(self.packet[1]!=0),"ts":timestamp}
            self.vehicleState["steer"]={"angle":int.from_bytes(self.packet[2:6],byteorder='big'),"ts":timestamp}
            self.vehicleState["motor"]={"speed":int.from_bytes(self.packet[6:10],byteorder='big',signed=True),"ts":timestamp}
            self.updated.add("EStop")
            self.updated.add("steer")
            self.updated.add("motor")
            return

        if (sensorType==Vehicle.SENSOR_TYPE_TURRET_DATA):
            self.vehicleState.setdefault("Turret",{}).update({sensorID:{"count":int.from_bytes(self.packet[0:4],byteorder='big'),
                                                   "value":int.from_bytes(self.packet[4:8],byteorder='big'),
                                                   "min":int.from_bytes(self.packet[8:12],byteorder='big'),
                                                   "max":int.from_bytes(self.packet[12:16],byteorder='big'),
                                                   "ts":timestamp}})
            self.updated.add("Turret")
            return

        if (sensorType==Vehicle.SENSOR_TYPE_WHEEL_ENCODER):
            self.vehicleState.setdefault("steer",{}).update({sensorID:{"ticks":int.from_bytes(self.packet[0:4],byteorder='big',signed=True),
                                                                       "ts": timestamp}})
            self.updated.add("steer")
            return
                                                             
        if (sensorType==Vehicle.SENSOR_TYPE_9D_SENSOR):
            self.vehicleState["9d"]={"acc":[int.from_bytes(self.packet[0:2],byteorder='big',signed=True),
                                            int.from_bytes(self.packet[2:4],byteorder='big',signed=True),
                                            int.from_bytes(self.packet[4:6],byteorder='big',signed=True)],
                                     "gyr":[int.from_bytes(self.packet[6:8],byteorder='big',signed=True),
                                            int.from_bytes(self.packet[8:10],byteorder='big',signed=True),
                                            int.from_bytes(self.packet[10:12],byteorder='big',signed=True)],
                                     "mag":[int.from_bytes(self.packet[12:14],byteorder='big',signed=True),
                                            int.from_bytes(self.packet[14:16],byteorder='big',signed=True),
                                            int.from_bytes(self.packet[16:18],byteorder='big',signed=True)],
                                     "ts": timestamp}
            self.updated.add("9d")
            return
        
        if (sensorType==Vehicle.SENSOR_TYPE_BATTERY_STATUS):
            self.vehicleState["batt"]={"temp":int.from_bytes(self.packet[0:2],byteorder='big'),
                                       "soc" :int.from_bytes(self.packet[2:4],byteorder='big'),
                                       "tte" :int.from_bytes(self.packet[4:6],byteorder='big'),
                                       "v"   :int.from_bytes(self.packet[6:8],byteorder='big'),
                                       "ts": timestamp}
            self.updated.add("batt")
            return
        
        if (sensorType==Vehicle.SENSOR_TYPE_REV_TICKS):
            self.vehicleState["revs"]={"ticks":int.from_bytes(self.packet[0:4],byteorder='big',signed=True),
                                       "ts": timestamp}
            self.updated.add("revs")
            return

        print(sensorType)
        raise(ProtocolException("Unhandled Sensor Type message"))

    # ======================================
    # STATE MACHINE FOR RECEIVING THE PACKET
    # ======================================
    class RxState(Enum):
        RX_IDLE         = 0
        RX_STX_L        = 1
        RX_VERSION      = 2
        RX_GETLEN       = 3
        RX_GETPACKET    = 4
        RX_GETCHECKSUMH = 5
        RX_GETCHECKSUML = 6

    def _crc16(self, input_data):
        crc_value = 0xffff

        for c in input_data:
            tmp = (crc_value>>8) ^ c
            tmp ^= tmp>>4
            crc_value = ((crc_value<<8)&0xFFFF)^((tmp<<12)&0xFFFF)^((tmp<<5)&0xFFFF)^(tmp&0xFFFF)
        return crc_value
        
    def _rxHandleIdle(self,j):
        if (Vehicle.STX_H!=j[0]):
            self.stats["duffSTX"]+=1
        else:
            self.rxState=self.RxState.RX_STX_L

    def _rxHandleSTXL(self,j):
        if (Vehicle.STX_L!=j[0]):
            self.stats["duffSTX"]+=1
            self.rxState=self.RxState.RX_IDLE
        else:
            self.rxState=self.RxState.RX_VERSION

    def _rxHandleVersion(self,j):
        if (Vehicle.VERSION!=j[0]):
            self.stats["badPkt"]+=1
            self.rxState=self.RxState.RX_IDLE
        else:
            self.rxState=self.RxState.RX_GETLEN

    def _rxHandleGetlen(self,j):
        self.rxingLen=j[0]
        self.PktLen=j[0]
        self.packet=bytearray()
        self.packetCrc=0
        self.rxState=self.RxState.RX_GETPACKET

    def _rxHandleGetpacket(self,j):
        self.packet.extend(j)
        self.rxingLen-=1
        if (self.rxingLen==0):
            self.rxState=self.RxState.RX_GETCHECKSUMH

    def _rxHandleGetChecksumH(self,j):
        self.packetCrc=j[0]<<8;
        self.rxState=self.RxState.RX_GETCHECKSUML

    def _rxHandleGetChecksumL(self,j):
        self.packetCrc|=j[0]
        if (self._crc16(self.packet)!=self.packetCrc):
            self.stats["badPkt"]+=1
        else:
            self.stats["goodPkt"]+=1
            self._parsePacket()
        self.rxState=self.RxState.RX_IDLE
        
    rxStateAction = {RxState.RX_IDLE        : _rxHandleIdle,
                     RxState.RX_STX_L       : _rxHandleSTXL,
                     RxState.RX_VERSION     : _rxHandleVersion,
                     RxState.RX_GETLEN      : _rxHandleGetlen,
                     RxState.RX_GETPACKET   : _rxHandleGetpacket,
                     RxState.RX_GETCHECKSUMH: _rxHandleGetChecksumH,
                     RxState.RX_GETCHECKSUML: _rxHandleGetChecksumL
                     }
    
    # ======================================
    # Commands to the Vehicle
    # ======================================

    def _sendCmd(self, sensorType, sensorID, cmdSeq):
        b=Vehicle.HEADER.copy()
        b.append(len(cmdSeq)+6)
        ts=int(time.time()*1000)
        b.extend([sensorType, sensorID,
                  (ts>>24)&0xFF,(ts>>16)&0xFF,(ts>>8)&0xFF,ts&0xFF])
        b.extend(cmdSeq)
        crc=self._crc16(b[4:])
        b.extend([(crc>>8)&0xFF,crc&0xFF])
        self.comm.write(b)

    def setSpeed(self,speed):
        if ((speed<-100) or (speed>100)):
            raise ProtocolError("Speed out of range")
        if (abs(speed)<10):
            speed=0
        self._sendCmd(Vehicle.CMD_TYPE_SETSPEED,0,[(speed>>8)&0xFF,speed&0xFF])

    def setServo(self,servo,proportion):
        if (servo>0):
            raise ProtocolError("Servo out of range")
        if ((proportion<0) or (proportion>1000)):
            raise("Proportion out of range")
        self._sendCmd(Vehicle.CMD_TYPE_SETSERVO,servo,[(proportion>>8)&0xFF,proportion&0xFF])

    def _ledCommand(self,ledcmd):
        if (ledcmd>Vehicle.CMD_LED_MAX):
            raise ProtocolError("Unknown LED command")
        self._sendCmd(Vehicle.CMD_TYPE_CMDLED,ledcmd,[])         

    def ledClear(self):
        self._ledCommand(Vehicle.CMD_LED_CLEAR)
        
    def ledPrint(self):
        self._ledCommand(Vehicle.CMD_LED_PRINT)

    def ledSet(self,ledList,command=CMD_LED_NONE):
        if (len(ledList)>12):
            raise ProtocolError("Too many LEDs in list")

        l=[]

        for led in ledList:
            # Deal with special case of 31n1 which has the leds in the wrong order 
            if (self.config["icarus31n1"]):
                l.extend([(3,4,0,1,0)[led[0]],led[1],led[2],led[3]])
            else:
                l.extend([led[0],led[1],led[2],led[3]])
        self._sendCmd(Vehicle.CMD_TYPE_SETLED,command,l)

    def mgmtEnterFlashMode(self):
        self._sendCmd(Vehicle.CMD_TYPE_MGMT,Vehicle.MGMT_CMD_FLASHUPDATE,Vehicle.SIGNATURE)
        
    # ======================================
    # Setup functions
    # ======================================

    def __init__(self, config):
        self.config=config
        errCount=0
        while 1:
            try:
                self.comm=serial.Serial(self.config["commport"], self.config["speed"], timeout=1)
                break

            except OSError as o:
                if (o.errno==2):
                    raise
                if (o.errno==16):
                    time.sleep(1)
                    errCount+=1
                    if (errCount>15):
                        raise(ProtocolException("Failed to open port"))
                else:
                    raise    
                
        self.stats={"duffSTX":0, "badPkt":0, "goodPkt":0, "badHeader":0}
        self.vehicleState={"timestamp":0}
        self.updated=set()
        self.rxState = self.RxState.RX_IDLE

    def getState(self):
        return self.vehicleState

    def getUpdated(self):
        retUpdated=list(self.updated)
        self.updated.clear()
        return retUpdated
        
    def doRead(self):
        while (self.comm.inWaiting()):
            j=self.comm.read(1)
            self.rxStateAction[self.rxState](self,j)

    def fileno(self):
        # This is needed by select to allow the object to particiate in the
        # select call
        return (self.comm.fileno())
