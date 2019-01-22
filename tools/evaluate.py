import math
f = open("log.txt")

a = 0
received_up=0
received_br=0

total_id = 500
max_num = [0] * (total_id + 1)
max_num1 = [0] * (total_id + 1)

start_pknum = [-1] * (total_id + 1)
total_start_pknum = 0

num_pk_up = 0
num_pk_down=0

delay_up_total =0
delay_up=0
delay_up_e2e=0
num_d_up=0;


delay_down=0
num_d_down=0;

even_line = []
odd_line = []
cnt = 0
o_idx = 1
invalid_line = 1
average = 0
send_down=0
counter_power =0
average_dc=0
power=0
re_tx =0

flag = 0

for line in f:

    if "initializing " in line: 
        flag = 1


    if "sendmsg " in line: 
        send_down +=1

    if "rxvs " in line:
        received_up +=1
        s_line = line.split(' ')
        print s_line
        num_id = int(s_line[8])

        # the # of packets at the client side
        max_num[num_id] = max(max_num[num_id], int(s_line[2])) # the # of packets client sent
        max_num1[num_id] = max(max_num1[num_id], int(s_line[3])) # the # of packets client received

        # set start packet num
        if (start_pknum[num_id] < 0 ) :
            start_pknum[num_id] = int(s_line[2])-1;

           
        #delay calculation (upstream)
        d = int(s_line[10]) - int(s_line[4])   #server_rx_time - client_send_time
        if (int(s_line[10]) >= int(s_line[4])):
            delay_up += d
            num_d_up += 1
            print d, int(s_line[10]), int(s_line[4]) , num_id, '.......o'
        else:
            print d, int(s_line[10]), int(s_line[4]) , num_id

        #delay calculation (downstream)
        d = int(s_line[6])
        if (d < 300000):
            delay_down += d
            num_d_down += 1

	
    if "ID:1	Data: " in line:
        cnt+=1
        if (cnt%2 == 0): # even
            even_line.append(line)
        else: # odd

            ## the first line in in here => odd_line[0]
            odd_line.append(line)     
    
           
#    if "ID:1	Power radio" in line: 
#        counter_power +=1
#        s_line = line.split(' ')
#        power += int(s_line[2])

    if "st 2-" in line: 
        re_tx +=1
        
        
for num in max_num:
    num_pk_up += num # the # of packets client sent
for num in max_num1:
    num_pk_down += num # the # of packets client received

for num in start_pknum:
  if (num > 0) :
      total_start_pknum += num # the sum of start packet num


num_pk_up -= total_start_pknum
    

#average_dc = power/counter_power



delay_up_e2e = float ((delay_up) / (num_d_up))
delay_down_e2e = float ((delay_down) / (num_d_down))

pdr_up= float(received_up) / (num_pk_up) * 100

pdr_down = float(num_pk_down)/(send_down)*100

print '===== Start Analyzing ====='


print '=========================================='

#print '', delay_total
print '\n=================PDR================='
print 'UPSTREAM PDR = ', round(pdr_up, 5), "%", "(", received_up, "/", num_pk_up, ")" , total_start_pknum
print 'DOWNSTREAM PDR = ', round(pdr_down, 5), "%", "(", num_pk_down, "/", send_down, ")"




print '\n================Latency================'
print 'UP: E2E Delay = ', round (delay_up_e2e, 5), "ms", "(",num_d_up,")"
print 'DOWN: E2E Delay = ', round (delay_down_e2e, 5), "ms", "(",num_d_down,")"

print '----------------------------------'
#print "Duty Cycle = ", average_dc , '%'
print "re-transmission time: ", re_tx
print '----------------------------------'


if (flag == 0) :
    print 'initial logs have been lost ... unvalid logs... ERROR !!!'

