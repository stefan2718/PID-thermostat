function [temp, uncertainty] = servoOnce(handle, temperature, period, duration)
% period should be in s, but passed in ms
  count = duration / period;
  sendVector(handle,[0 temperature (period*1000) count]); % send command to Arduino
  
  temp = waitVector(handle, 60);
  uncertainty = waitVector(handle);
  temp = temp*1e-6;
  uncertainty = uncertainty*1e-6;
end