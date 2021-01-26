clear all; clc;close all;
% filename = '.\cornerPoint.txt';
% [xf,yf,h,s,xw,yw,zw]=textread(filename,'%n%n%n%n%n%n%n',81);
% subplot(121);
% scatter3(xw,yw,zw,'o');
% %plot(xf,yf,'o');
% hold on;
%%
filename = '.\Result.txt';
[xf2,yf2,h2,s2,xw2,yw2,zw2]=textread(filename,'%n%n%n%n%n%n%n',81);
scatter3(xw2,yw2,zw2,'o');
hold on;

dis = (xw2(4)-xw2(5))^2;
dis = dis + (yw2(4)-yw2(5))^2;
dis = dis + (zw2(4)-zw2(5))^2;
dis = sqrt(dis);
%%
% filename = '.\PointsCalib.txt';
% [h1,s1,x0,y0,z0,d1,d2,d3]=textread(filename,'%n%n%n%n%n%n%n%n',81);
% 
% % »­Í¼
% for i=1:81
%     t=-100:1:100;
%     xx=x0(i)+d1(i)*t;
%     yy=y0(i)+d2(i)*t;
%     zz=z0(i)+d3(i)*t;
%     plot3(xx,yy,zz);
%     hold on;
% end
%%
filename = '.\Result.txt';
[xf,yf,h,s,xw2,yw2,zw2]=textread(filename,'%n%n%n%n%n%n%n',81);
fx = 2522.8299306623685;
fy = 2521.8714910641474;
u0 = 630.10602870497394;
v0 = 500.25175311068062;

% for i=1:81
%     syms t;
%     n = 9*h(i)+s(i)+1;
%     a = ((xf(i)-u0)/(yf(i)-v0))*fy/fx;
%     equa = a-((x0(n)+d1(n)*t)/(y0(n)+d2(n)*t));
%     t = solve(equa);
%     
%     xc = x0(n)+d1(n)*t;
%     yc = y0(n)+d2(n)*t;
%     zc = z0(n)+d3(n)*t;
%     
%     plot3(xc,yc,zc,'o');
%     hold on;
%     
% end
