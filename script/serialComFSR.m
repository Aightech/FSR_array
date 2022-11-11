%% Script to received data data from FSRray
device = serialport("/dev/ttyACM0",500000);%connect to the arduino ( select the right path: COM1, COM2 ... on windows)
pause(1)%wait a bit


n=32;% size on the n*n array to measure

dt=[0 0];%array conataining 2 timestamps: [when request received; when request sent] 
vec = zeros(256, (1));%array containing the force values

%To close the communication with arduino when closing the plot
global running;
running=1;
figure('CloseRequestFcn',@my_closereq);


% While plotting window is open
while running
    write(device,n,"uint8");%send request with the size of the array to record
    dt = read(device,2,"uint32");%get timestamps
    vec = read(device,n*n,"uint16");%get forces array

    array = reshape(vec,n,n); % reshape data

    %plot the data
    surf(array);
    zlim([0,10000]);
    view([20 70])
    drawnow;

end

%closing everything when the plot windows is closed
clear device;
function my_closereq(src,event)
    global  running;
    running = 0;
    delete(gcf);
end