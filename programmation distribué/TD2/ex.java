class Compteur extends Thread{
    static final int NITER = 1000000;
    static int compteur=0;

    public void run(){
        for(int i=0 ; i<NITER;i++){
            compteur ++;
        }
    }
}

class CompteurTest{
    public static void main(String[] args) throws InteruptedExecption{
        
    }
}