function [xhat, SigmaX] = dummy_kf(s)
%DUMMY_KF  One step of a linear Kalman filter on random data.
%   Used only as a computational load: s states, one scalar measurement,
%   an s x s matrix inverse, covariance prediction and update.
%   The values are random; the result has no physical meaning.
%
%   [xhat, SigmaX] = dummy_kf(30)

if nargin < 1
    s = 30;
end

Ad     = rand(s, s);
Bd     = rand(s, 1);
Cd     = rand(1, s);
Dd     = 0;
xhat   = rand(s, 1);
u      = rand;
z      = rand;
SigmaX = rand(s, s);
SigmaW = rand(s, s);
SigmaV = rand;                 % measurement noise (scalar)

Ad = inv(Ad);                  % deliberate O(s^3) matrix inverse

% Step 1a: state prediction
xhat = Ad * xhat + Bd * u;
% Step 1b: covariance prediction
SigmaX = Ad * SigmaX * Ad' + SigmaW;
% Step 1c: predicted output
zhat = Cd * xhat + Dd * u;
% Step 2a: gain
L = SigmaX * Cd' / (Cd * SigmaX * Cd' + SigmaV);
% Step 2b: state update
xhat = xhat + L * (z - zhat);
% Step 2c: covariance update
SigmaX = (eye(s) - L * Cd) * SigmaX;
end
