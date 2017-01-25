/***************************************************
  Fuzzy Example

  by Tomás de Camino Beck
  www.funcostarica.org

  License:

  This example was created by Tomàs de Camino Beck (tomas@funcostarica.org)
  and is released under an open source MIT license.  See details at:
    http://opensource.org/licenses/MIT



  /***************************************************/
//We will use a structured data type to create label type def
//in C++ struct is a public object.
struct fuzzyDef {
  float mina = 0;
  float minb = 0;
  float maxa = 0;
  float maxb = 0;
  float centroid = 0;
  float (*membership)(fuzzyDef, float);//pointer to membership function
  float miu = 0.0; //to store current membership or fuzzy weight for defuzzification
};

//to create the entire domain of discourse for a fuzzy variable
//with n sets
struct fuzzyDomain {
  fuzzyDef* fset ;
  int nset;
};

/********************************************************/

#include <Servo.h>
Servo myservo;

fuzzyDomain accel, outC;



void setup() {

  myservo.attach(9);

  Serial.begin(9600);

  accel.nset = 3;
  accel.fset = new fuzzyDef[3];

  //creates a standard domain with nset=3 fuzzy sets
  setDomain(&accel, -10, 10);



  // creates output control fuzzy domain
  outC.nset = 3;
  outC.fset = new fuzzyDef[3];
  setDomain(&outC, 180);

  pinMode(A1, INPUT);


}

void loop() {
  // put your main code here, to run repeatedly:
  int val = analogRead(A1);
  //  long m=micros();
  truthDegree(&accel, val);

  // weightedAverage(&outC);
  Serial.print(accel.fset[0].mina);
  Serial.print(",");
  Serial.print(accel.fset[0].maxa);
  Serial.print(",");
  Serial.print(accel.fset[1].mina);
  Serial.print(",");
  Serial.print(accel.fset[1].maxa);
  Serial.print(",");
  Serial.print(accel.fset[2].mina);
  Serial.print(",");
  Serial.println(accel.fset[2].maxa);


  // Serial.println(micros()-m);

  delay(300);
}

/*****fuzzy membership functions*****/
/* they receibe the fuzzyDef as argument to get parameters */

float triangle(float t) {
  //creates a triangle from -1 to 1
  return max(1 - abs(t), 0);
}

float triangle(fuzzyDef fset, float x) {
  //triangle function between a min and max
  float mid = (fset.maxa - fset.mina) / 2;
  if (x < fset.mina) return 0;
  if (x > fset.maxa) return 0;
  return triangle((x - mid) / (fset.maxa - fset.mina) * 2 );
}


float increasing(fuzzyDef fset, float x) {
  if (x < fset.mina)return 0;
  if (x > fset.maxa)return 1;
  return (x - fset.mina) / (fset.maxa - fset.mina);
}

float decreasing(fuzzyDef fset, float x) {
  if (x < fset.mina)return 1;
  if (x > fset.maxa)return 0;
  return (fset.maxa - x) / (fset.maxa - fset.mina);
}

float trapezoid(fuzzyDef fset, float x) {
  if (x < fset.mina || x > fset.maxb)return 0;
  if (x > fset.maxa && x < fset.minb)return 1;
  if (x >= fset.mina && x <= fset.maxa)return increasing(fset, x);
  if (x >= fset.minb && x <= fset.maxb)return decreasing(fset, x);
}


/************ Fuzzy Operations ****************/
/**********************************************/

//function that return the truth of the membership given de parameter v
//is stores de value in miu for use in other functions
float truthDegree(fuzzyDef *fset, float v) {
  return fset->miu = fset->membership(*fset, v);;

}

//Fuzzy union definition
float fuzzyUnion(fuzzyDef *set1, fuzzyDef *set2, float v) {
  float degree1 = truthDegree(set1, v);
  float degree2 = truthDegree(set2, v);
  if (degree1 >= degree2) return degree1;
  else return degree2;
}

//Fuzzy intersection definition
float fuzzyIntersection(fuzzyDef *set1, fuzzyDef *set2,  float v) {
  float degree1 = truthDegree(set1, v);
  float degree2 = truthDegree(set2, v);
  if (degree1 <= degree2) return degree1;
  else return degree2;
}

//return true if current miu value > 0
//use for if statements to define fuzzy rules
boolean isMember(fuzzyDef *set1) {
  return set1->miu > 0;
}

//to set the centroid value for weighted average
void setCentroid(fuzzyDef *set1) {
  if (set1->minb > 0) {
    set1->centroid = (set1->maxa + set1->minb) / 2;
  }
  else set1->centroid = (set1->mina + set1->maxa) / 2;
}


/*********fuzzy domain functions ********/

//set miu in a fuzzy domain, all to 0
void resetDomain(fuzzyDomain *domain) {
  for (int i = 0; i < sizeof(domain->fset); i++) {
    domain->fset[i].miu = 0;
  }
}

void truthDegree(fuzzyDomain *domain, float v) {
  for (int i = 0; i < domain->nset; i++) {
    truthDegree(&domain->fset[i], v);
  }
}

//sets a standard fuzzy domain with n membership functions
void setDomain(fuzzyDomain *domain, float maxvalue) {
  if (domain->nset > 1) {
    float d = maxvalue / (domain->nset+1);
    int s = domain->nset - 1;
    domain->fset[0].mina = d;
    domain->fset[0].maxa = 2 * d;
    domain->fset[0].centroid = (domain->fset[0].mina + domain->fset[0].maxa) / 2;
    domain->fset[0].membership = &decreasing;
    for (int i = 1; i < s; i++) {
      domain->fset[i].mina = i * d;
      domain->fset[i].maxa = (i + 2) * d;
      domain->fset[i].centroid = (domain->fset[i].mina + domain->fset[i].maxa) / 2;
      domain->fset[i].membership = &triangle;
    }
    domain->fset[s].mina = maxvalue - d * 2;
    domain->fset[s].maxa = maxvalue - d;
    domain->fset[s].centroid = (domain->fset[s].mina + domain->fset[s].maxa) / 2;
    domain->fset[s].membership = &increasing;
  }

}

//sets a standard fuzzy domain with n membership functions
//sets a standard fuzzy domain with n membership functions
void setDomain(fuzzyDomain *domain, float minvalue, float maxvalue) {
  if (domain->nset > 1) {
    float d = (maxvalue - minvalue) / (domain->nset+1);
    int s = domain->nset - 1;
    domain->fset[0].mina = minvalue + d;
    domain->fset[0].maxa = minvalue + 2 * d;
    domain->fset[0].centroid = (domain->fset[0].mina + domain->fset[0].maxa) / 2;
    domain->fset[0].membership = &decreasing;
    for (int i = 1; i < s; i++) {
      domain->fset[i].mina = minvalue + i * d;
      domain->fset[i].maxa = minvalue + (i + 2) * d;
      domain->fset[i].centroid = (domain->fset[i].mina + domain->fset[i].maxa) / 2;
      domain->fset[i].membership = &triangle;
    }
    domain->fset[s].mina = maxvalue - d * 2;
    domain->fset[s].maxa = maxvalue - d;
    domain->fset[s].centroid = (domain->fset[s].mina + domain->fset[s].maxa) / 2;
    domain->fset[s].membership = &increasing;
  }
}


/*defuzzifying functions
   weighted average for speed ans simplicity
   it works well if fuzzy membership functions are simetrical
*/

float weightedAverage(fuzzyDomain *domain) {
  float wsum = 0;
  float sum = 0;
  for (int i = 0; i < domain->nset; i++) {
    wsum += domain->fset[i].miu * domain->fset[i].centroid;
    sum += domain->fset[i].miu;
  }
  if (sum > 0) return wsum / sum;
  else return 0;
}




