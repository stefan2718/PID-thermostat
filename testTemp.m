function [vector] = testTemp(handle)
  sendVector(handle, 1); % send command to Arduino
  vector = waitVector(handle);
  if vector(1) > 0
    error('Arduino apparently indicated a problem');
  end
end