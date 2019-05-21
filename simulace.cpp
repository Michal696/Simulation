#include "simlib.h"
#include <iostream>
#include <time.h>
#include <stdlib.h>

#define DEBUG 0

#define DLZKA_SIMULACE 8*60  // 8 hodin
#define DLZKA_VYROBY 2.5
#define CAS_PRIPRAVY_MASA 0.5
#define CAS_PRIPRAVY_VELA_ZELENINY 6
#define CAS_PRIPRAVY_MALO_ZELENINY 5
#define CAS_PLACENI 1/3
#define MAX_RADA 15
#define VELKA_RADA 10
#define MALA_RADA 5
#define MALO_ZELENINY 40
#define KRITICKY_MALO_ZELENINY 10
#define MAX_PRIPRAVENEJ_ZELENINY 80
#define MNOZSTVO_NAREZANEHO_MASA 5

#define PAUZA 0.1

/////EXPERIMENT
///// v ruzne faze dne , kolik potrebujeme zamestnancu
#define GENERACIA_ZAKACNIKOV 1.6  // v noci 1.2, ve dne 1.6
#define ZAMESTNANCI 1
#define PRIEMERNY_ZAROBOK_NA_OBJEDNAVKU 27.14
#define PLAT_VEDUCEHO_NA_HODINU 150
#define PLAT_ZAMESTNANCA 110



uint pripravene_maso = 0;
uint pripravena_zelenina = MAX_PRIPRAVENEJ_ZELENINY;
uint opustil_neuspesne = 0;
uint opustil_uspesne = 0;
uint fronta_na_placeni = 0;
uint fronta_na_vyrobu = 0;
uint lidi_v_prodejne = 0;

// Facility musi mit Seize aj Release v tom istom procesy

Facility PripravnaZeleniny("Priprava zeleniny");
Facility PripravnaMasa("Priprava masa");
Store Zamestnanec("Zamestnanec", ZAMESTNANCI);
Facility Vstup("Vstupne dvere");
Facility Pokladna("Pokladna");
Facility Pristup_k_masu("Maso");
Facility Pristup_k_zelenine("Zelenina");
Facility Pristup_ku_kase("Kasa");
Facility Pristup_k_fronte_na_placeni("fronta_na_placeni");
Facility Pristup_k_fronte_na_vyrobu("fronta_na_vyrobu");



class Vyroba : public Process{
public:
    //  Vyroba() : Process() {};

     void Behavior(){
        while (1){

            Seize(Pristup_k_zelenine);
            if((pripravena_zelenina <= KRITICKY_MALO_ZELENINY) && !PripravnaZeleniny.Busy()){ // p == 3 doplnenie vela zeleniny
                Release(Pristup_k_zelenine);
                if(DEBUG) printf("PripravaVelaZeleniny\n");
                // (new PripravaVelaZeleniny())->Activate();
                /////////////////////////////////////////////0
                    Enter(Zamestnanec, 1);

                    Seize(PripravnaZeleniny);

                    Wait(CAS_PRIPRAVY_MALO_ZELENINY); // priprava zeleniny 5 minut
                    Seize(Pristup_k_zelenine);
                    pripravena_zelenina += (MAX_PRIPRAVENEJ_ZELENINY - MALO_ZELENINY);
                    Release(Pristup_k_zelenine);
                    Release(PripravnaZeleniny);

                    Leave(Zamestnanec, 1);
                /////////////////////////////////////////////1
            }else{
                Release(Pristup_k_zelenine);
                pridanie_masa:
                Seize(Pristup_k_masu); // X1
                Seize(Pristup_k_fronte_na_vyrobu); // X2
                if((pripravene_maso != 0) && (fronta_na_vyrobu != 0)){ // p == 2 VyrobaObjednavky
                    pripravene_maso--;
                    Release(Pristup_k_masu);
                    Seize(Pristup_k_zelenine);
                    pripravena_zelenina--;
                    Release(Pristup_k_zelenine);
                    fronta_na_vyrobu--;
                    if(DEBUG) printf("VyrobaObjednavky\n");
                    Release(Pristup_k_fronte_na_vyrobu); // X2

                    // (new VyrobaObjednavky())->Activate(); // Wait pripadne placeni
                    /////////////////////////////0
                        Enter(Zamestnanec, 1);
                        Wait(DLZKA_VYROBY);

                        Seize(Pristup_k_fronte_na_placeni);
                        if((fronta_na_placeni != 0) && (!Pristup_ku_kase.Busy())){
                            // pokud nevybral penize nekdo jiny, tak vybere on

                            fronta_na_placeni--;
                            Seize(Pristup_ku_kase);
                            Release(Pristup_k_fronte_na_placeni);
                            Wait(CAS_PLACENI); // placeni 20s
                            Release(Pristup_ku_kase);
                        }else{
                            Release(Pristup_k_fronte_na_placeni);
                        }

                        Seize(Vstup);
                        lidi_v_prodejne--;
                        opustil_uspesne++;
                        Release(Vstup);
                        Leave(Zamestnanec, 1);

                    //////////////////////////1
                }else{// p == 1 vyroba masa
                    if((pripravene_maso == 0) && (fronta_na_vyrobu != 0) && !PripravnaMasa.Busy()){ //
                        Release(Pristup_k_fronte_na_vyrobu);
                        Release(Pristup_k_masu);
                        if(DEBUG) printf("PripravaMasa\n");
                        // (new PripravaMasa())->Activate();
                        ////////////////////////////////////////////0
                            Enter(Zamestnanec, 1);
                            Seize(PripravnaMasa);
                            Wait(CAS_PRIPRAVY_MASA);
                            Seize(Pristup_k_masu);
                            pripravene_maso += MNOZSTVO_NAREZANEHO_MASA;
                            Release(Pristup_k_masu);
                            Release(PripravnaMasa);
                            Leave(Zamestnanec, 1);
                        ////////////////////////////////////////////1
                        goto pridanie_masa;
                    }else{
                    // if(DEBUG) Wait(4);
                        Release(Pristup_k_fronte_na_vyrobu);
                        Release(Pristup_k_masu); // X1
                        Seize(Pristup_k_fronte_na_placeni);
                        if((fronta_na_placeni != 0) && (!Pristup_ku_kase.Busy())){// p == 1 zamestnanec jde ke kase
                                fronta_na_placeni--;
                                Release(Pristup_k_fronte_na_placeni);
                                if(DEBUG) printf("Placeni\n");
                                // (new Placeni())->Activate();
                                ////////////////////////////////////////////////0
                                    Enter(Zamestnanec, 1);

                                    Seize(Pristup_ku_kase);

                                    Wait(CAS_PLACENI);
                                    Release(Pristup_ku_kase);

                                    Leave(Zamestnanec, 1);
                                ////////////////////////////////////////////////1
                        }else{ // p == 0 MALO_ZELENINY
                            Release(Pristup_k_fronte_na_placeni);
                            Seize(Pristup_k_zelenine);
                            if((pripravena_zelenina <= MALO_ZELENINY) && (!PripravnaZeleniny.Busy())) {
                                Release(Pristup_k_zelenine);
                                if(DEBUG) printf("PripravaMaloZeleniny\n");
                                // (new PripravaMaloZeleniny())->Activate();
                                /////////////////////////////////////////////0
                                    Enter(Zamestnanec, 1);

                                    Seize(PripravnaZeleniny);

                                    Wait(CAS_PRIPRAVY_MALO_ZELENINY); // priprava zeleniny 5 minut
                                    Seize(Pristup_k_zelenine);
                                    pripravena_zelenina += (MAX_PRIPRAVENEJ_ZELENINY - MALO_ZELENINY);
                                    Release(Pristup_k_zelenine);
                                    Release(PripravnaZeleniny);

                                    Leave(Zamestnanec, 1);
                                /////////////////////////////////////////////1
                            }
                            else{
                                Release(Pristup_k_zelenine);

                                Enter(Zamestnanec, 1);
                                Wait(PAUZA);
                                //pripadne pridanie nizsej priority
                                if(DEBUG) printf("Nic nerobenie\n");
                                Leave(Zamestnanec, 1);

                            }
                        }
                    }
                }
            }



            if(DEBUG){
                printf("Maso: %d\n", pripravene_maso);
                printf("Zelenina: %d\n", pripravena_zelenina);
                printf("opustil_uspesne: %d\n", opustil_uspesne);
                printf("lidi_v_prodejne: %d\n", lidi_v_prodejne);
                printf("fronta_na_vyrobu: %d\n", fronta_na_vyrobu);
                printf("fronta_na_placeni: %d\n", fronta_na_placeni);
                printf("opustil_neuspesne: %d\n", opustil_neuspesne);
                printf("Celkem: %d\n\n", opustil_neuspesne + lidi_v_prodejne + opustil_uspesne);
            }
        } // WHILE
    }

};

class Zakaznik: public Process{
public:
    void Behavior(){

        Seize(Vstup);
        if(lidi_v_prodejne == MAX_RADA){
            opustil_neuspesne++;
            if(DEBUG) printf("GENERATOR: opustil_neuspesne\n");
        }else{
            if(lidi_v_prodejne >= VELKA_RADA){
                if(Random() <= 0.8){
                    opustil_neuspesne++;
                    if(DEBUG) printf("GENERATOR: opustil_neuspesne\n");
                }else{
                    Seize(Pristup_k_fronte_na_vyrobu);
                    Seize(Pristup_k_fronte_na_placeni);
                    fronta_na_vyrobu++;;
                    fronta_na_placeni++;
                    lidi_v_prodejne++;
                    if(DEBUG) printf("GENERATOR: Prisiel zakaznik\n");
                    Release(Pristup_k_fronte_na_placeni);
                    Release(Pristup_k_fronte_na_vyrobu);
                }
            }else{
                if(lidi_v_prodejne >= MALA_RADA){
                    if(Random() <= 0.3){
                        opustil_neuspesne++;
                        if(DEBUG) printf("GENERATOR: opustil_neuspesne\n");
                    }else{
                        Seize(Pristup_k_fronte_na_vyrobu);
                        Seize(Pristup_k_fronte_na_placeni);
                        fronta_na_vyrobu++;;
                        fronta_na_placeni++;
                        lidi_v_prodejne++;
                        if(DEBUG) printf("GENERATOR: Prisiel zakaznik\n");
                        Release(Pristup_k_fronte_na_placeni);
                        Release(Pristup_k_fronte_na_vyrobu);
                    }
                }else{
                    Seize(Pristup_k_fronte_na_vyrobu);
                    Seize(Pristup_k_fronte_na_placeni);
                    fronta_na_vyrobu++;;
                    fronta_na_placeni++;
                    lidi_v_prodejne++;
                    if(DEBUG) printf("GENERATOR: Prisiel zakaznik\n");
                    Release(Pristup_k_fronte_na_placeni);
                    Release(Pristup_k_fronte_na_vyrobu);
                }
            }
        }
        Release(Vstup);
    }
};

// generuje zakazniku
class	GeneratorZakazniku : public Event {
public:
    GeneratorZakazniku(){}
    ~GeneratorZakazniku(){}

	void	Behavior() {
		(new Zakaznik())->Activate();
		Activate(Time+Exponential(GENERACIA_ZAKACNIKOV));
	}
};

int main(){
    RandomSeed(time(NULL));
    // srand(time(NULL));
    // SetOutput("simulacia.txt");
    Init(0, DLZKA_SIMULACE+1);

    (new GeneratorZakazniku())->Activate();
    for(int i = 0 ; i < ZAMESTNANCI  ; i++){// pridat
        (new Vyroba())->Activate();
    }


    Run();

    // nase vypisky
    //////////////INPUT////////////////////////
    printf("Pocet zamestnancu: %d\n", ZAMESTNANCI);
    if(GENERACIA_ZAKACNIKOV == 1.2) printf("Smena: Noc\n");
    else   printf("Smena: Den\n");



    //////////////////////OUTPUT/////////////////////
    printf("opustil_uspesne: %d\n", opustil_uspesne);
    printf("fronta_na_vyrobu: %d\n", fronta_na_vyrobu);
    printf("fronta_na_placeni: %d\n", fronta_na_placeni);
    printf("opustil_neuspesne: %d\n", opustil_neuspesne);
    printf("Celkem opustilo: %d\n", opustil_neuspesne + opustil_uspesne);
    printf("Zisk: %.2f Kč\n", ((opustil_uspesne*(PRIEMERNY_ZAROBOK_NA_OBJEDNAVKU)) - ((DLZKA_SIMULACE/60)*(ZAMESTNANCI-1)*PLAT_ZAMESTNANCA + (DLZKA_SIMULACE/60)*PLAT_VEDUCEHO_NA_HODINU)));



    return 0;

}
