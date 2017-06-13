function the = ggx_sample(beta, n)
        t = rand(n, 1);
        the = atan(beta*sqrt(t./(1 - t)));
end