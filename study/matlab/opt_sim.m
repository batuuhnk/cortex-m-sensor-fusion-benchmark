function r = opt_sim()
%OPT_SIM  Small constrained optimization by gradient descent with a
%   quadratic penalty. Used only as a scalar, branch-heavy computational load:
%   maximize the radius r of a sphere that must fit in a cube of edge L.

L = 10.0;      % cube edge
r = 0.2;       % initial radius
mu = 100.0;    % penalty weight
alpha = 0.1;   % step size
max_iter = 2000;
tol = 1e-10;

for k = 1:max_iter
    viol  = max(0, r - L/2);
    dJ    = -1 + 2*mu*viol;          % derivative of J = -r + mu*viol^2
    r_new = max(0, r - alpha*dJ);
    if abs(r_new - r) < tol
        r = r_new;
        break;
    end
    r = r_new;
end
end
