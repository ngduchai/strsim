clc;
clear;
close all;
data = csvread('H:\data.txt');
maxC = max(data(:, 1));
maxK = max(data(:, 2));
[C, K] = meshgrid(0:maxC/100:maxC, 0:maxK/100:maxK);
avg = griddata(data(:, 1), data(:, 2), data(:, 3), C, K);
tail99 = griddata(data(:, 1), data(:, 2), data(:, 4), C, K);
figure;
mesh(C, K, avg);
xlim([0 maxC]);
ylim([0 maxK]);
xlabel("C");
ylabel("K");
zlabel("Latency (us)");
title("Average Latency");
figure;
mesh(C, K, tail99);
xlim([0 maxC]);
ylim([0 maxK]);
xlabel("C");
ylabel("K");
zlabel("Latency (us)");
title("99-Percentile Latency");
