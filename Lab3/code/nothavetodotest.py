import os

base = './main ../test/nothavetodo/test'
count = 2
while(count != 3):
    combination = base+str(count)+'> ../test/nothavetodo/test'+str(count)+'.ir'
    print('\n','test'+str(count))
    os.system(combination)
    count += 1
