//ex2

public interface IPi extends Remote{
    public double compute(int iter)throws remote exeption;
}

public class serveur implements IPi extends unicastRemoteObject{
    @java.lang.Override
    public double compute(int iter) throws remoteExecption {
        double sum=0;
        ....
        return sum;
    }
}
public static void main(Strin arg[]){
    try {
        serveur erveur = new serveur();
        registry.bind("PiServeur", serveur);
        system.ent.println("serveur ready");
    }catch(...){...}
}

public class client{
    public static void main(string args[]){
        try{
            IPi ipi = Narning.lookup("rmi://localhost:/PiServer");
            double sum=ipi.compute (100000);
            systeme.out.println(sum);
        }catch(...){...}
    }
}

// ex 2 Version2
public interface Ipi extends remote{
    public void computer (int inter, object odc)..
}
public interface odc extends remote{
    public void notifier(double res)...
}
public class odc impl implements odc extends unicastRemoteObject{
    double result=0;
    public void notifier(double res){
        this.result=0;
    }
    public double getResult(){
        return this.result;
    }
}
public clas Serveur implemnts Ipi extends unicatRemoteObject{
    public void computer (int inet), Object (odc)...
        {
            double sum=0;
        ...
        odc.notifier (sum);
        }
        public static void main(String args[]){
        // meme chose
        }
}
public class client extends thread{
    public static ODC odc;
    public void run(){
        system.out println("icent fait")
    }
    public static void main (strin arg[]){
        try {
            odc= nex odc oac Impl();
            Client client= new client();
            client.start();
            Ipi ipi =Narming.lookup("PiServeur");
            ipi.computer(100000,cdc);
            while{odc.getResultat()==0{
                Thread.sleep(100){
                    systeme.println(odc.getrsult());
                    UnicastRemote.objectunexportTObject(odc,t)
                }
            }
        }
    }
}

