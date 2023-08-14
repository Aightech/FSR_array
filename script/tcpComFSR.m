%% Script to received data data from FSRray

device = serialport("/dev/ttyACM0",500000);%connect to the arduino ( select the right path: COM1, COM2 ... on windows)
pause(1)%wait a bit

% device = tcpclient("192.168.127.254",5000)
% pause(1)%wait a bit

nb_layer=1;
n=16;% size on the n*n array to measure


cmd = n;
if(nb_layer==2)
    cmd=cmd+0x80;
end

dt=[0 0];%array conataining 2 timestamps: [when request received; when request sent] 
prev_dt=dt;
vec = zeros(256, (1));%array containing the force values

%To close the communication with arduino when closing the plot
global running;
running=1;
figure('CloseRequestFcn',@my_closereq); 

vals=[];


% While plotting window is open
while running
    write(device,cmd,"uint8");%send request with the size of the array to record
    prev_dt=dt;
    dt = read(device,2,"uint32");%get timestamps
    disp(dt(1)-prev_dt(1))

    vec = read(device,nb_layer*n*n,"uint16");%get forces array
    %array = reshape(vec,nb_layer*n,n); % reshape data

    %plot the data
%     maxz=600;
%     hSurface=surf(array);
%     caxis([0,maxz]);
%     colorbar;
%     zlim([0,maxz]);
%     view([20 70]);
    if size(vals,1)==0
        vals = vec;
    else
        vals = [vals; vec];
    end
% plot(dt(1),array(8,8),"o");
% hold on;
   % drawnow;

end
v = cast(vals,"double");
v=v-mean(v(1:200,:),1);

%% plot

% for i=4*16:12*16
%     plot(lowpass(v(:,i), 10, 800));
%     hold on;
% end
% hold off
%%

%closing everything when the plot windows is closed
clear device;
function my_closereq(src,event)
    global  running;
    running = 0;
    delete(gcf);
end