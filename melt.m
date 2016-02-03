function [temp, uncertainty, temp2, uncertainty2] = melt(handle, period, duration)
% period should be in s, but passed in ms
  count = duration / period;
  temp = zeros(1,count);
  uncertainty = zeros(1,count);
  temp2 = zeros(1,count);
  uncertainty2 = zeros(1,count);
  
  sendVector(handle,[4 (period*1000) count]); % send command to Arduino

  for i = 1:count
    vector = waitVector(handle);
    if vector(1) > 0
      error('Arduino apparently indicated a problem');
    end
    temp(i) = vector(2)*1e-2;
    uncertainty(i) = vector(3)*1e-2;
    temp2(i) = vector(4)*1e-2;
    uncertainty2(i) = vector(5)*1e-2;
  end
end