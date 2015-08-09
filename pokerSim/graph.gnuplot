reset


#set terminal post enhanced color
#set output "graph.eps"


#fac(x) = (int(x)==0) ? 1.0 : int(x) * fac(int(x)-1.0)

#choose(n,k) = (int(n) == int(k) | int(k)==0 )? 1 : choose( n-1, k-1) + choose( n-1, k )

choose(n,k) = (int(n) == int(k) | int(k)==0 )? 1 : int(n) * choose( n-1, k-1) / int(k)

binom(n,p,x) = choose(n,x) * p**x * (1-p)**(n-x)
			 
cumBinom(n,p,x) = (int(x)<0) ? 0 : binom(n,p,x) + cumBinom(n,p,x-1)

set boxwidth 0.5
set style fill solid

set key off
set contour

set grid ytics front lt 0 lw 2 lc rgb '#000000'
#set grid mytics front lt 1 lw 1 lc rgb '#000000'

set grid noxtics
n = 90

numTargetHands = 78 + 60
p = numTargetHands/1326.0
#p=0.5
ymax = 1.1 * binom( n, p, floor((n+1)*p) ) #mode of binomial PDF used
ymax = 1
plotMax = 20

set yrange [0:ymax]
set xrange [0:plotMax]
set sample plotMax+1
set xtics 1
set ytics 0.05
#set mytics 10
#show mytics

plot cumBinom(n,p,x) with boxes lt 1 lc rgb '#DD6600', binom(n,p,x) with boxes lt 1 lc rgb '#00CCCC'
#plot choose(10,x) with boxes