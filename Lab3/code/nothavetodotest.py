import os

base = './main ../test/nothavetodo/test'
count = 1
while(count != 5):
    combination = base+str(count)
    print('\n','test'+str(count))
    os.system(combination)
    count += 1
