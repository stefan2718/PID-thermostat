function [temp, uncertainty] = servoA(handle, temperature, period, duration)
  count = duration/period;
  temp = zeroes(1,count);
  uncertainty = zeros(1,count);
  
  for i = 1:count
      sendVector(handle,[1 temperature period]); % send command to Arduino
      vector = waitVector(handle);
      if vector(1) > 0
        error('Arduino apparently indicated a problem');
      end
      temp(i) = vector(2)*1e-6;
      uncertainty(i) = vector(3)*1e-6;
  end
  sendVector(handle, 2);
end