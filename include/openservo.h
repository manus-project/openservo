#ifndef __OPENSERVO_LIB
#define __OPENSERVO_LIB

#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include <memory>

using namespace std;

#define SERVO_MAX_SPACE 0x80

namespace openservo {

class ServoBus;

class Servo {
friend ServoBus;
public:

  virtual ~Servo();

  // commands
  bool reset();
  bool enable();
  bool disable();
  bool registersCommit();
  bool registersRestore();
  bool registersDefault();

  int getType();
  int getSubType();
  int getVersion();
  int getFlags();
  int getTimer();
  int getPosition();
  int getVelocity();
  int getPower();
  int getPWM();

  int getSeekPosition();
  int getSeekVelocity();
  int getCurveInVelocity();
  int getCurveOutVelocity();

  void setSeekPossition(int value);
  void setSeekVelocity(int value);
  void setCurveInVelocity(int value);
  void setCurveOutVelocity(int value);

  int getAddress();
  int getPidDeadband();
  int getPidGainProportional();
  int getPidGainDerivative();
  int getPidGainIntegral();
  int getPwmFreqDivider();
  int getMinSeek();
  int getMaxSeek();
  int getReverseSeek();
  int getReverseSeekTo();
  int getServoID();
  int getCurrentCutOff();
  int getCurrentSoftCuttOff();

  // Protected write methods

  void unlock();

  void setAddress(int address); // change servo message bus ID
  void setPidDeadband(int deadband);
  void setPidGain(int proportional, int derivative, int integral);
  void setPwmFreqDivider(int value);
  void setMinSeek(int value);
  void setMaxSeek(int value);
  void setReverseSeek(int value);
  void setReverseSeekTo(int value);
  void setServoID(int value);
  void setCurrentCutOff(int value);
  void setCurrentSoftCuttOff(int value);


  bool update(bool full = false);

protected:

  Servo(ServoBus* bus, int address);

private:

  int read2B(int address);
  int read1B(int address);

  void write2B(int address, int value);
  void write1B(int address, int value);

  bool command(unsigned char cmd);

  ServoBus* bus;

  bool responsive;  // TODO: not used at the moment
  bool updated;
  bool locked;

  unsigned char data[SERVO_MAX_SPACE];
  bool local[SERVO_MAX_SPACE];

};

typedef std::shared_ptr<Servo> ServoHandler;

class ServoBus {
friend Servo;
public:

  ServoBus();
  ~ServoBus();

  bool open(const string& port);
  bool close();
  bool update(bool full = false);

  int scan(bool force = false);

  ServoHandler get(int i);
  int size();

protected:

  bool send(unsigned char address, unsigned char data_address, unsigned char* data, int data_lenght); 
  bool receive(unsigned char address, unsigned char data_address, unsigned char* data, int data_lenght);

private:

  void* handle;
  vector<ServoHandler> servos;

  bool exists(int address);  
  ServoHandler find(int address);
  int cleanupServos();
  int addServos();

};

}

#endif
