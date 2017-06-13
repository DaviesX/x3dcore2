function y = ggx(phi, beta)
        beta2 = beta*beta;
        
        sin_phi = sin(phi);
        cos_phi = cos(phi);
        cos_phi2 = cos_phi.*cos_phi;
        tan_phi = tan(phi);
        f = beta2 + tan_phi.*tan_phi;
        y = 2*beta2.*sin_phi.*cos_phi./(cos_phi2.*cos_phi2.*f.*f);
end