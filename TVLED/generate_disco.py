import random

_N_LED = 545

all_violet = list( range( 0, 174 ) )
all_beige = list( range( 174, 174+83 ) )
all_yellow = list( range( 174+83, 174+83+114 ) )
all_cyan = list( range( 174+83+114, 174+83+114+174 ) )

res = []

for i in range(100) : 

    this_res = []
    this_res += random.sample( all_violet, 2 ) 
    this_res += random.sample( all_beige, 2 ) 
    #this_res += random.sample( all_yellow, 2 ) 
    #this_res += random.sample( all_cyan, 2 ) 
    res.append( this_res)

print (res)

for r in res : 
    print ('{'+','.join([str(x) for x in r]) + '},')


