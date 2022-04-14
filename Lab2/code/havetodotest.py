import os

base = './main ../test/havetodo/test'
count = 1
while(count != 18):
    combination = base+str(count)
    print('\n','test'+str(count))
    os.system(combination)
    count += 1
