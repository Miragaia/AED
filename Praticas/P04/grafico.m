A = load('data.txt');

f=2;
n = A(A(:,2)==f ,1);
r = A(A(:,2)== f ,3);
t = A(A(:,2)== f ,4);
figure;plot(n,r,"-*"), title('r')
r2t = n.*(n+1)/2;
hold on
plot(n,r2t, 'r-o')

figure;plot(n,r2t, 'r-o'), title('time');