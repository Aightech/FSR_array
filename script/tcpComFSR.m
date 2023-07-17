%% Script to received data data from FSRray
device = tcpclient("192.168.127.254",5000)
pause(1)%wait a bit

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


% While plotting window is open
while running
    write(device,cmd,"uint8");%send request with the size of the array to record
    prev_dt=dt;
    dt = read(device,2,"uint32");%get timestamps
    disp(dt(1)-prev_dt(1))

    if(nb_layer==2)
        vec = read(device,2*n*n,"uint16");%get forces array
        array = reshape(vec,2*n,n); % reshape data
    else
        vec = read(device,n*n,"uint16");%get forces array
        array = reshape(vec,n,n); % reshape data
    end

    %plot the data
    maxz=600;
    hSurface=surf(array);
    caxis([0,maxz]);
    colorbar;
    zlim([0,maxz]);
    view([20 70]);
    drawnow;

end

%closing everything when the plot windows is closed
clear device;
function my_closereq(src,event)
    global  running;
    running = 0;
    delete(gcf);
end