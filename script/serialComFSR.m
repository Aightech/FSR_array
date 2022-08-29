device = serialport("/dev/ttyACM0",500000)
pause(1)
disp('Now receiving data...');
vec = zeros(256, (1));

n=32;
global running;
running=1;

figure('CloseRequestFcn',@my_closereq);
while running
    write(device,n,"uint8");
    dt_prev=dt(1);
    dt = read(device,2,"uint32");
    fprintf('dt=%d \tdt2=%d \tfreq=%f\n',dt(1)-dt_prev,dt(2)-dt(1), 1000000./(dt(1)-dt_prev));
    vec = read(device,n*n,"uint16");

    % get data from the inl
    % and display it
    %fprintf('%.2f\t',vec);
    surf(reshape(vec,n,n));
    zlim([0,1000]);
    view([20 70])
    drawnow;

end

clear device;

function my_closereq(src,event)
    global  running;
    running = 0;
    delete(gcf);
end