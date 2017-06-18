phi=0:0.01:pi/2;
beta = 0.1;

D = ggx(phi, beta);

figure, plot(phi, D);

the = ggx_sample(beta, 10000);
figure, hist(the, 20);