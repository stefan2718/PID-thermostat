int sizeOfInt;
double lastError = 0;
const int numReadings = 10;
const int maxPower = 200; //maybe
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
  Serial.begin(115200);
  Serial.print("McGillPhysicsUgrad339"); /* This is a big magic number --- MATLAB waits for this on connect */
  Serial.write(sizeOfInt = sizeof(int)); /* different models of Arduino have different size integers */
  Serial.write(strlen(timestamp));
  Serial.print(timestamp);
  pinMode(13,OUTPUT);
  pinMode(powerOut,OUTPUT);
}
void testTemp(int *cmdVector){
  for (int i=1; i<=10; i++){
    cmdVector[i] = analogRead(thermIn);
    delay(2);
  }
  cmdVector[0] = 0;
  sendVector(11, cmdVector);
}

void servo(int *cmdVector){
  int setTemp = cmdVector[1];
  int period = cmdVector[2]; 
  int many = cmdVector[3];
  int allTemp[many];
  int allUnct[many];
  
  for (int k=0; k<many; k++){
    long time1 = millis();
    double temp[numReadings];
    double powerSet;
    double sum = 0;
    double squareSum = 0;
    
    for (int i = 0; i<numReadings ; i++){
      temp[i] = analogRead(thermIn)*(500.0 / 1023.0); 
      // need conversion here?
      // float voltage= sensorValue * (5.0 / 1023.0);
      sum += temp[i];
      delay(1);
    }
    double meanTemp = sum/(double)numReadings;
  
    for (int i = 0; i<numReadings ; i++){
      squareSum += (meanTemp - temp[i]) * (meanTemp - temp[i]);
    }
    double uncertainty = sqrt(squareSum/(double)((numReadings-1)*numReadings));
    
    if (meanTemp <= setTemp-10){
      analogWrite(powerOut, maxPower);
    } else if (meanTemp < setTemp+10 && meanTemp > setTemp-10){
      double error = (setTemp - meanTemp)/20;
      powerSet = (0.5 + error + ((error + lastError)/2 * (period/1000)) - ((error - lastError)/(period/1000)));
                                 /*        derivative term           */   /*         integral term          */
      if (powerSet>1) { powerSet = 1; }
      if (powerSet<0) { powerSet = 0; }
      analogWrite(powerOut, (int)(powerSet*maxPower));
      lastError = error;
    } else {
      analogWrite(powerOut, 0);    
    }
    
    allTemp[k] = (int)(meanTemp*1000000); // shift these up?
    allUnct[k] = (int)(uncertainty*1000000);
    delay(period-(millis()-time1));
  }
  sendVector(many, allTemp);
  delay(500);
  sendVector(many, allUnct);
  analogWrite(powerOut, 0);
}

void loop() {
  int cmdVector[16]; /* this vector will be used for communication */
  int count;
  if (Serial.available()) { /* If MATLAB has sent a command, go get it */
    count = waitVector(cmdVector);
    if (cmdVector[0] == 0){
      servo(cmdVector);
    } else if (cmdVector[0] == 1){
      testTemp(cmdVector);
    }
  } 
}
