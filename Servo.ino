int sizeOfInt;
const double numReadings = 15.0;
const int numm = (int) numReadings;
const int maxPower = 255; //maybe
const int thermIn = A0;
const int powerOut = 10;
const int fanOut = 11;

void sendVector (int count, int *vector) {
  int i;
  void *ptr = &count;
  void *dptr = &vector[0];
  Serial.write((uint8_t*)ptr, sizeof(int));
  Serial.write((uint8_t*)dptr,count*sizeof(int));
}

int waitVector (int *vector) {
  int count = 0;
  int i, j;
  while (Serial.available() < sizeOfInt);
  for (i = 0; i < sizeOfInt; i++) {
    count += (Serial.read() << (i*8));
  }
  for (j = 0; j < count; j++) {
    vector[j] = 0;
    while (Serial.available() < sizeOfInt);
    for (i = 0; i < sizeOfInt; i++) vector[j] += (Serial.read() << (i*8));  
  }
  return count;
}

void setup() {                                                                 
  const char *timestamp = __DATE__ ", " __TIME__;                              
  int i;                                                                       
  Serial.begin(115200);                                                        
  Serial.print("McGillPhysicsUgrad339"); /* This is a big magic number --- MATLAB waits for this on connect */                                                
  Serial.write(sizeOfInt = sizeof(int)); /* different models of Arduino have different size integers */                                                       
  Serial.write(strlen(timestamp));                                             
  Serial.print(timestamp);                                                     
  pinMode(13,OUTPUT);
  pinMode(powerOut,OUTPUT);  
  while (Serial.available() < 4) digitalWrite(13,(millis()/125)%2); /* bounce back MATLAB's magic number */                                                   
  for (i = 0; i<4; i++) Serial.write(Serial.read());                           
}        

void testTemp(int *cmdVector){
  for (int i=1; i<=10; i++){
    cmdVector[i] = analogRead(thermIn);
    delay(1);
  }
  cmdVector[0] = 0;
  sendVector(11, cmdVector);
}
void testFan(int *cmdVector){
  analogWrite(fanOut, maxPower);
  delay(4000);
  analogWrite(fanOut, 0);
  cmdVector[0] = 0; // success
  sendVector(1, cmdVector);
}

void servo(int *cmdVector){
  int setTemp = cmdVector[1];
  int period = cmdVector[2]; 
  int many = cmdVector[3];
  double lastError = 0;
  
  for (int k=0; k<many; k++){
    long time1 = millis();
    double temp[numm];
    double powerSet;
    double sum = 0;
    double squareSum = 0;
    
    for (int i = 0; i<numm ; i++){
      temp[i] = analogRead(thermIn)*(98.0/203.3); 
      sum += temp[i];
      delay(1);
    }
    double meanTemp = sum/numReadings;
  
    for (int i = 0; i<numm ; i++){
      squareSum += (meanTemp - temp[i]) * (meanTemp - temp[i]);
    }
    double uncertainty = sqrt(squareSum/((numReadings-1)*numReadings));
    
    if (meanTemp <= setTemp-10){
      powerSet = 1;
      analogWrite(powerOut, maxPower);
    } else if (meanTemp < setTemp+10 && meanTemp > setTemp-10){
      double error = (setTemp - meanTemp)/20;
      powerSet = (0.5 + error + ((error + lastError)/2 * (period/1000)) - ((error - lastError)/(period/1000)));
                                 /*        integral term           */   /*         derivative term          */
      if (powerSet>1) { powerSet = 1; }
      if (powerSet<0) { powerSet = 0; }
      analogWrite(powerOut, (int)(powerSet*maxPower));
      lastError = error;
    } else {
      powerSet = 0;
      analogWrite(powerOut, 0);    
    }
    
    cmdVector[0] = 0;
    cmdVector[1] = (int)(meanTemp*100);
    cmdVector[2] = (int)(uncertainty*100);
    cmdVector[3] = (int)(powerSet*100);
    sendVector(4, cmdVector);
    delay(period-(millis()-time1));
  }
  analogWrite(powerOut, 0);
}
void melt(int *cmdVector){
  int period = cmdVector[1]; 
  int many = cmdVector[2];
  
  for (int k=0; k<many; k++){
    long time1 = millis();
    
    double temp[numm];
    double sum = 0;
    double squareSum = 0;
    
    double temp2[numm];
    double sum2 = 0;
    double squareSum2 = 0;
    
    for (int i = 0; i<numm ; i++){
      temp[i] = analogRead(thermIn)*(98.0/203.3); 
      temp2[i] = analogRead(A1)*(98.0/203.3);
      sum += temp[i];
      sum2 += temp2[i];
      delay(1);
    }
    double meanTemp = sum/numReadings;
    double meanTemp2 = sum2/numReadings;
  
    for (int i = 0; i<numm ; i++){
      squareSum += (meanTemp - temp[i]) * (meanTemp - temp[i]);
      squareSum2 += (meanTemp2 - temp2[i]) * (meanTemp2 - temp2[i]);
    }
    double uncertainty = sqrt(squareSum/((numReadings-1)*numReadings));
    double uncertainty2 = sqrt(squareSum2/((numReadings-1)*numReadings));
    
    cmdVector[0] = 0;
    cmdVector[1] = (int)(meanTemp*100);
    cmdVector[2] = (int)(uncertainty*100);
    cmdVector[3] = (int)(meanTemp2*100);
    cmdVector[4] = (int)(uncertainty2*100);
    sendVector(5, cmdVector);
    delay(period-(millis()-time1));
  }
}

void loop() {
  int cmdVector[16]; /* this vector will be used for communication */
  int count;
  if (Serial.available()) { /* If MATLAB has sent a command, go get it */
    count = waitVector(cmdVector);
    switch (cmdVector[0]){
      case 0: 
        servo(cmdVector);
        break;
      case 1:
        testTemp(cmdVector);
        break;
      case 2:
        testFan(cmdVector);
        break;
      case 3:
        analogWrite(powerOut, 0);
        break;
      case 4:
        melt(cmdVector);
        break;
    }
  } 
}
