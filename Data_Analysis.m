filename = fopen('csvData.csv', 'r');
%M = textscan(filename,'%d%d%d%d%d%d%d%d%d%d%d%d%d%d%c%c%c','Delimiter',',')
M = textscan(filename,'%d %d %d %d %d %d %d %d %d %d %d %d %d %d %c %c %c','Delimiter',',');
%M = csvread('csvData.csv', 0, 0, [0, 0, 100000, 14])

numberOfLines = 4614430;
newNumberOfLine = 4614430;

fclose(filename);

%%

for superman = 1:numberOfLines
    if M{1, 5}(superman) ~= 11401
        for batman = 1:17
            M{1, batman}(superman) = 0;
        end
        newNumberOfLine = newNumberOfLine - 1;
    end 
end

%%
M{1, 7} = 3.2206*(double(M{1, 7})*(5/255)) - 0.086; %convert from steps to voltage (V)
M{1, 8} = 0.1116*(double(M{1, 8})*(5/255)) - 0.0009; %convert from steps to current (A)
time = M{1, 5};
linecount = M{1, 1};
voltage = M{1, 7};
current = M{1, 8};
long = M{1, 2};
lat = M{1, 3};
alt = M{1, 4};
datapoints = linspace(1, numberOfLines, numberOfLines);



%%


%time = M{1, 5};

% figure(1);
% %linecount = M{1, 1};
% plot(time, linecount, '.');
% xlabel('UTC time (s)') % x-axis label
% ylabel('Line count') % y-axis label
% 
% 
% figure(2);
% %voltage = M{1, 7};
% plot(time, voltage, '.');
% xlabel('UTC time (s)') % x-axis label
% ylabel('Voltage') % y-axis label
% 
% 
% figure(3);
% %current = M{1, 8};
% plot(time, current, '.');
% xlabel('UTC time (s)') % x-axis label
% ylabel('Current') % y-axis label
% 
% 
% figure(4);
% %long = M{1, 2};
% %lat = M{1, 3};
% %alt = M{1, 4};
% plot3(long, lat, alt, '.');
% xlabel('Longitude') % x-axis label
% ylabel('Latitude') % y-axis label
% zlabel('Altitude') % y-axis label
% 
% figure(5);
% %linecount = M{1, 1};
% plot(time, linecount, '.');
% xlabel('UTC time (s)') % x-axis label
% ylabel('Line count') % y-axis label
% 
% figure(6);
% %datapoints = linspace(1, 4614430, 4614430);
% plot(datapoints, time, '.');
% xlabel('UTC time (s)') % x-axis label
% ylabel('Line count') % y-axis label
% 
% figure(7);
% %datapoints = linspace(1, 4614430, 4614430);
% plot(datapoints, voltage, '.');
% xlabel('Line count') % x-axis label
% ylabel('Voltage') % y-axis label
% 
% figure(8);
% %datapoints = linspace(1, 4614430, 4614430);
% plot(datapoints, alt, '.');
% xlabel('Line count') % x-axis label
% ylabel('Altitude') % y-axis label

figure(1);
for kirk = 1:9 
    subplot(3,3,kirk);
    plot(datapoints, M{1,kirk}, '.');
    xlabel('Sample Number') % x-axis label
    
    switch kirk
        case 1
            ylabel('Line Counter')
        case 2
            ylabel('Longitude (deg*10^-5)')
        case 3
            ylabel('Latitude (deg*10^-5)')
        case 4
            ylabel('Altitude (cm)')
        case 5
            ylabel('UTC time (s)')
        case 6
            ylabel('Thermistor (centi-celsius)')
        case 7
            ylabel('Voltage (V)')
        case 8
            ylabel('Current (A)')
        case 9
            ylabel('Sensor 1')
    end
end

figure(2);
for kirk = 10:14 
    subplot(3,3,kirk-9);
    plot(datapoints, M{1,kirk}, '.');
    xlabel('Sample Number') % x-axis label
    
    switch kirk
        case 1
            ylabel('Sensor 2')
        case 2
            ylabel('Sensor 3')
        case 3
            ylabel('Sensor 4')
        case 4
            ylabel('Sensor 5')
        case 5
            ylabel('Sensor 6')
    end
end


figure(3);
for kirk = 1:9 
    subplot(3,3,kirk);
    plot(time, M{1,kirk}, '.');
    xlabel('Time (s)') % x-axis label
    
    switch kirk
        case 1
            ylabel('Line Counter')
        case 2
            ylabel('Longitude (deg*10^-5)')
        case 3
            ylabel('Latitude (deg*10^-5)')
        case 4
            ylabel('Altitude (cm)')
        case 5
            ylabel('UTC time (s)')
        case 6
            ylabel('Thermistor (centi-celsius)')
        case 7
            ylabel('Voltage (V)')
        case 8
            ylabel('Current (A)')
        case 9
            ylabel('Sensor 1')
    end
end

figure(4);
for kirk = 10:14 
    subplot(3,3,kirk-9);
    plot(time, M{1,kirk}, '.');
    xlabel('Time (s)') % x-axis label
    
    switch kirk
        case 1
            ylabel('Sensor 2')
        case 2
            ylabel('Sensor 3')
        case 3
            ylabel('Sensor 4')
        case 4
            ylabel('Sensor 5')
        case 5
            ylabel('Sensor 6')
    end
end

