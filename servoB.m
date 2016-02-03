function [temp, uncertainty, power, vector] = servoB(handle, temperature, period, duration)
% period should be in ms
  count = duration / period;
  temp = zeros(1,count);
  uncertainty = zeros(1,count);
  power = zeros(1,count);
  
  sendVector(handle,[0 temperature (period*1000) count]); % send command to Arduino
  
  for i = 1:count
      vector = waitVector(handle);
      if vector(1) > 0
        error('Arduino apparently indicated a problem');
      end
      temp(i) = vector(2)*1e-2;
      uncertainty(i) = vector(3)*1e-2;
      power(i) = vector(4)*1e-2;
  end
end