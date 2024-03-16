%% Script to received data data from FSRray
device = serialport("/dev/ttyACM0",500000);%connect to the arduino ( select the right path: COM1, COM2 ... on windows)
pause(1)%wait a bit

nb_layer=2;
n=16;% size on the n*n array to measure


cmd = n;
if(nb_layer==2)
    cmd=cmd+0x80;
end

dt=[0 0];%array conataining 2 timestamps: [when request received; when request sent] 
vec = zeros(256, (1));%array containing the force values

%To close the communication with arduino when closing the plot
global running;
running=1;
figure('CloseRequestFcn',@my_closereq);

x=1:16;

% While plotting window is open
while running
    write(device,cmd,"uint8");%send request with the size of the array to record
    prev_dt=dt;
    dt = read(device,2,"uint32");%get timestamps
    disp(dt(1)-prev_dt(1));
    disp(dt);

    if(nb_layer==2)
        vec = read(device,2*n*n,"uint16");%get forces array
        array = reshape(vec,2*n,n); % reshape data
    else
        vec = read(device,n*n,"uint16");%get forces array
        array = reshape(vec,n,n); % reshape data
    end
    %cmd = read(device,1,"uint8");
    %cmd 


    %array(2,:)=array(3,:);
    %array(18,:)=array(17,:);

    %plot the data
    maxz=800;
%    hSurface=surf(array);
    hSurface=surf(array(1:16,:));
    hold on;
    surf(300+array(17:end,:));
    hold off;
    caxis([0,maxz]);
    colorbar;
    zlim([0,maxz]);
    view([135 20]);

%plot(dt(1),array(8,8));

%     k=8;
%     plot(x,array(k,1:16));
%     hold on;
%     plot(x,100+array(k+16,1:16));
%     hold off;
    drawnow;

end

%closing everything when the plot windows is closed
clear device;
function my_closereq(src,event)
    global  running;
    running = 0;
    delete(gcf);
end