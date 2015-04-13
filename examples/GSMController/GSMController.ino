// This example will turn on the GSM Shield and wait for either a Call or SMS
// Calls are answered and DTMF recorded, terminated by # or timeout.  SMS is dispatched as is.
// Within the DTMF/SMS: For each 1, turn on LED for 1 second.  For each 2 turn off LED for 1 second.

#include "SIM900.h"
#include <SoftwareSerial.h>
#include "sms.h"
#include "call.h"

#define ledPin 13

CallGSM call;
SMSGSM sms;
byte stat=0;

void setup() 
{
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("GSM Shield testing.");

  // Start configuration of GSM shield with baudrate.
  // For http uses is recommended to use 4800 or slower.
  if (gsm.begin(19200)) {
    Serial.println("\nstatus=READY");
  } else {
    Serial.println("\nstatus=IDLE");
  }
};

void loop() 
{
  Serial.println("Wait for call.");
  stat = call.CallStatus();
  if(stat == CALL_INCOM_VOICE) {
    Serial.println("Answer incoming call!");

    // Detect DTMF until #
    call.SetDTMF(DTMF_DETECT_ON);

    // Answer
    call.PickUp();

    int maxDTMF = 20;
    char bufferDTMF[maxDTMF+1];
    int indexDTMF = 0;
    int timeouts = 0;

    // Loop waiting for either DTMF tone or timeout
    do {
      Serial.println("Listen for DTMF:");
      char dtmf = call.DetDTMF();
      
      Serial.print("Received:");
      Serial.println(dtmf);
      
      if (dtmf == '-') {
        // - means timeout, 10 in a row and we hang up
        timeouts++;
      } else if (dtmf == '#') {
        // # received, stop listening for DTMF
        bufferDTMF[indexDTMF++] = dtmf;
        break;
      } else {
        // append next DTMF received
        bufferDTMF[indexDTMF++] = dtmf;
        timeouts=0;
      }
      
    } while (indexDTMF < maxDTMF && timeouts < 10);  // After 10 timeouts, stop listening for DTMF (several seconds)

    // Terminate the DTMF string
    bufferDTMF[indexDTMF] = '\0';
    
    // Hang up
    call.HangUp();

    // Stop detecting DTMF
    call.SetDTMF(DTMF_DETECT_OFF);
    
    Serial.print("DTMF Tones Received: ");
    Serial.print(bufferDTMF);
    Serial.print(" (Length: ");
    Serial.print(sizeof(bufferDTMF));
    Serial.println(")");
        
    // Do something
    dispatch(bufferDTMF);
  }
  
  Serial.println("Wait for SMS.");
  int sms_index = sms.IsSMSPresent(SMS_ALL);

  // Index = SMS slot #
  if(sms_index > 0) {
    char phone_num[20];
    char sms_text[160];
    int sms_status = sms.GetSMS(sms_index, phone_num, sms_text, 160);

    Serial.print("SMS#");
    Serial.print(sms_index);
    Serial.print(" from  ");
    Serial.print(phone_num);
    Serial.print(" : ");
    Serial.println(sms_text);

    dispatch(sms_text);
    
    Serial.print("Delete SMS.");
    sms.DeleteSMS(sms_index);
  }

  delay(3000);
};

void dispatch(char *command) {
  divider();
  Serial.print("Command Received: ");
  Serial.println(command);
  
  // If 1 then turn on LED, wait 1 sec
  // If 2 then turn off LED, wait 1 sec
  do {
    char next = *command++;
    Serial.print("->");
    Serial.println(next);

    if (next == '1') {
      digitalWrite(ledPin, HIGH);
      delay(1000);
    } else if (next == '2') {
      digitalWrite(ledPin, LOW);
      delay(1000);
    } else if (next == '#') {
      // end DTMF command
      break;
    } else if (next == '\0') {
      // end SMS command
      break;
    }
  } while (true);

  divider();
};

void divider() {
  for (int i=0;i<50;i++) {
    Serial.print('=');
  }
  Serial.println("");
}
