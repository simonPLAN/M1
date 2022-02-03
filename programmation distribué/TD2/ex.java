// ex 1
class Compteur extends Thread{
    static final int NITER = 1000000;
    static int compteur=0;
    static object obj=new Object();

    public void run(){
        for(int i=0 ; i<NITER;i++){
            synchronized (obj){
                compteur ++; // session critique
            }
        }
    }
}


class CompteurTest{
    public static void main(String[] args) throws InteruptedExecption{
        // nombre de thread default 4
        int nbThreads=(args.length<1)?4: Integer.parseInt(args[0])
                Compteur[] tc = new Compteur[nbThreads];
        // cree et demare les threads
        for(int i =0;i<nbThreads;i++){
            tc[i]=new Compteur();
            tc[i].start();
        }

        // attents ques les thread sois terminer
        for(int i = 0;i<nbThreads;i++){
            tc[i].join();

            //compare la valeur de compterur avec la valeur attendu
            if(Compteur.compteur!=Compteur.NITER*nbThreads)
                systeme.out.println("Erreure ! competeur = " + Compteur.compteur);
            else
                systeme.out.println("ok ! competeur = " + Compteur.compteur);


        }
    }
}

// ex 2

f(0)=0
f(1)=1
f(2)=1
f(n+2)=f(n+1)+f(n)


public class Fibonnacci extends Thread{
    public static void main(String[] args){
        Thread t = new Fibonnacci(s); // calcule de f5
        T.start();
        try{
            t.join();
        }catch (InteruptedExecption){...}
        systeme.out.println(t.resultat);
    }
    public Fibonnacci(long val){
        this.val=val;
    }
}

public void run(){
    if(this.val==0) this.resultat=0;
    else
        if(this.val==1)this.resultat=1;
        else{
            Thread t1=new Fibonnacci(this.val-1);
            Thread t2=new Fibonnacci(this.val-2);
            t1.start();
            t2.start();
            try{
                t1.join();
                t2.join();
            }catch.....
            this.resultat=t1.resultat+t2.resultat;
        }
}

// exercice 3



class Exemple extends Thread{
    public void run(){
        Systeme.out.print("exemple");
    }
}
class Multithread extends Thread{
    Thread t;
    public Multithread(Thread t ){ this.t=t;}
    public void run(){
        try{this.t.join();}catch ...
        Systeme.out.print("Multithread");
    }
}
class Mt2 {
    public static void main(String[] args) {
        Exemple.exemple = new Exemple();
        Multithread multi = new Multithread(exemple);
        mult.start()
        exemple.start();
        try {
            multi.join();
            {
            catch ...
            }
            Systeme.out.println("simple");
        }
    }
}
// exercice 4

class exemple extends Thread{
    object verrou;
    public exemple(Object verou){this.verrou=verrou;}
    public void run(){
    synchronized ((verrou){
        systeme.out.print("exemple");
        verrrou.notify;
        verrou.set();
       }
    }
}
class Multithread extends Thread{
    object verrou;
    public Multithread(object verrou){
    this.verrou=verrou
    }
    public void run(){
        synchronized (verrou);
        while(verrou.get()==0){
                verrou.wait();
        }
        //attente passive
            try{verrou.wait()
    }

}
}
public Verrou{
    int passage=0;
    public int get(){
        return passage;
    }
    public void set(){
        passage=1;
    }
}