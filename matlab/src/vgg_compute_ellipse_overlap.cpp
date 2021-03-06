#include <mex.h>
#include <math.h>

// This mex function implements the equivalent of:
//
//   function y = example_mat(x)
//   y = 2 * x + 1;
//   return

// [y, z] = foo(a, b, c)
//
//  nrhs = 3
//  a : prhs[0]
//  b : prhs[1]
//  c : prhs[2]
//
//  nlhs = 2
//  y : plhs[0]
//  z : plhs[1]

void mexFunction(int nlhs,       mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
  // must have single double array input.
  if (nrhs != 3) { printf("in nrhs != 3\n"); return; }
  if (mxGetClassID(prhs[0]) != mxDOUBLE_CLASS) { printf("input must be a double array\n"); return; }
  if (mxGetClassID(prhs[1]) != mxDOUBLE_CLASS) { printf("input must be a double array\n"); return; }

  // must have single output.
  if (nlhs != 4) { printf("out nlhs != 4\n"); return; }

  // get size of x.
  size_t const num_dims1 = mxGetNumberOfDimensions(prhs[0]);
  size_t const *dims1 = mxGetDimensions(prhs[0]);
  size_t const num_dims2 = mxGetNumberOfDimensions(prhs[1]);
  size_t const *dims2 = mxGetDimensions(prhs[1]);
  size_t const num_dims3 = mxGetNumberOfDimensions(prhs[2]);
  size_t const *dims3 = mxGetDimensions(prhs[2]);
  if(dims1[0]<9 || dims2[0]<9 || dims1[0]!=dims2[0] || dims3[0]!=1){ printf("dims != 9\n"); return; }

  // create new array of the same size as x.

  size_t *odim = new size_t[2];
  odim[0]=dims2[1];
  odim[1]=dims1[1];
  plhs[0] = mxCreateNumericArray(2, odim, mxDOUBLE_CLASS, mxREAL);
  plhs[1] = mxCreateNumericArray(2, odim, mxDOUBLE_CLASS, mxREAL);
  plhs[2] = mxCreateNumericArray(2, odim, mxDOUBLE_CLASS, mxREAL);
  plhs[3] = mxCreateNumericArray(2, odim, mxDOUBLE_CLASS, mxREAL);

  // get pointers to beginning of x and y.
  double const *feat1 = (double *)mxGetData(prhs[0]);
  double const *feat2 = (double *)mxGetData(prhs[1]);
  double const *flag = (double *)mxGetData(prhs[2]);
  double       *over_out = (double *)mxGetData(plhs[0]);
  double       *mover_out = (double *)mxGetData(plhs[1]);
  double       *desc_out = (double *)mxGetData(plhs[2]);
  double       *mdesc_out = (double *)mxGetData(plhs[3]);

  /* av: tover is temporary space */

  double *feat1a = new double[9];
  double *feat2a = new double[9];
  double *tdesc_out = new double[dims2[1]*dims1[1]];
  double *tover_out = new double[dims2[1]*dims1[1]];

  int common_part=(int)flag[0];

  for(int j=0;j<dims1[1];j++){
    for (int i=0; i<dims2[1]; i++){
      over_out[j*dims2[1]+i]=100.0;
      desc_out[j*dims2[1]+i]=1000000.0;
    }
  }

  // printf("%f %f\n",flag[0],flag[1]);
  // total number of elements in arrays.
  /*int total = 1;
  for (int i=0; i<num_dims1; ++i){
    total *= dims1[i];
    printf("feat1 %d  %d  \n",num_dims1, dims1[i]);
  }
  */

  double max_dist,fac,dist,dx,dy,bna,bua,descd,ov;
  for(int j=0,f1=0;j<dims1[1];j++,f1+=dims1[0]){
    max_dist=sqrt(feat1[f1+5]*feat1[f1+6]);
    if(common_part>0)fac=30/max_dist;
    else if(common_part==0) fac=3;
    else fac=1 ; /* av */

    /*
    if (j=339-1) {
    mexPrintf("#### %g %g %g %g %g %g %g %g\n",
              feat1[f1],
              feat1[f1+1],
              feat1[f1+2],
              feat1[f1+3],
              feat1[f1+4],
              feat1[f1+5],
              feat1[f1+6],
              fac) ;
    }
    */
    max_dist=max_dist*4;
    fac=1.0/(fac*fac);
    feat1a[2]=fac*feat1[f1+2];
    feat1a[3]=fac*feat1[f1+3];
    feat1a[4]=fac*feat1[f1+4];
    feat1a[7] = sqrt(feat1a[4]/(feat1a[2]*feat1a[4] - feat1a[3]*feat1a[3]));
    feat1a[8] = sqrt(feat1a[2]/(feat1a[2]*feat1a[4] - feat1a[3]*feat1a[3]));
    for (int i=0,f2=0; i<dims2[1]; i++,f2+=dims1[0]){


      //compute shift error between ellipses
      dx=feat2[f2]-feat1[f1];
      dy=feat2[f2+1]-feat1[f1+1];
      dist=sqrt(dx*dx+dy*dy);
      /*
      if (j == 0) {
        mexPrintf("over with %d ? %g\n", i, dist) ;
      }
      */


      if(dist<max_dist){
        feat2a[2]=fac*feat2[f2+2];
        feat2a[3]=fac*feat2[f2+3];
        feat2a[4]=fac*feat2[f2+4];
        feat2a[7] = sqrt(feat2a[4]/(feat2a[2]*feat2a[4] - feat2a[3]*feat2a[3]));
        feat2a[8] = sqrt(feat2a[2]/(feat2a[2]*feat2a[4] - feat2a[3]*feat2a[3]));
        //find the largest eigenvalue
        double maxx=ceil((feat1a[7]>(dx+feat2a[7]))?feat1a[7]:(dx+feat2a[7]));
        double minx=floor((-feat1a[7]<(dx-feat2a[7]))?(-feat1a[7]):(dx-feat2a[7]));
        double maxy=ceil((feat1a[8]>(dy+feat2a[8]))?feat1a[8]:(dy+feat2a[8]));
        double miny=floor((-feat1a[8]<(dy-feat2a[8]))?(-feat1a[8]):(dy-feat2a[8]));

        double mina=(maxx-minx)<(maxy-miny)?(maxx-minx):(maxy-miny);
        double dr=mina/50.0;
        bua=0;bna=0;int t1=0,t2=0;
        //compute the area
        for(double rx=minx;rx<=maxx;rx+=dr){
          double rx2=rx-dx;t1++;
          for(double ry=miny;ry<=maxy;ry+=dr){
            double ry2=ry-dy;
            //compute the distance from the ellipse center
            double a=feat1a[2]*rx*rx+2*feat1a[3]*rx*ry+feat1a[4]*ry*ry;
            double b=feat2a[2]*rx2*rx2+2*feat2a[3]*rx2*ry2+feat2a[4]*ry2*ry2;
            //compute the area
            if(a<1 && b<1)bna++;
            if(a<1 || b<1)bua++;
          }
        }
        ov=100.0*(1-bna/bua);
        if (j == 0) {
          //mexPrintf("over with %d: %g\n", i, 1-ov/100.0) ;
        }
        tover_out[j*dims2[1]+i]=ov;
        mover_out[j*dims2[1]+i]=ov;
        //printf("overlap %f  \n",over_out[j*dims2[1]+i]);return;
      }else {
        tover_out[j*dims2[1]+i]=100.0;
        mover_out[j*dims2[1]+i]=100.0;
        ov=100.0 ;
      }
      /*
      if (i == 339-1 && j==3-1) {
        mexPrintf("\n#### %g %g %g %g %g %g %g %g\n",
                  feat1[f1],
                  feat1[f1+1],
            feat1[f1+2],
            feat1[f1+3],
            feat1[f1+4],
            feat1[f1+5],
            feat1[f1+6],
            fac) ;
        mexPrintf("#### %g %g %g\n", dist, max_dist, ov) ;
      }*/

      descd=0;
      for(int v=9;v<dims1[0];v++){
        descd+=((feat1[f1+v]-feat2[f2+v])*(feat1[f1+v]-feat2[f2+v]));
      }
      descd=sqrt(descd);
      tdesc_out[j*dims2[1]+i]=descd;
      mdesc_out[j*dims2[1]+i]=descd;
    }
  }

  double minr=100;
  int mini=0;
  int minj=0;
  do{
    minr=100;
    for(int j=0;j<dims1[1];j++){
      for (int i=0; i<dims2[1]; i++) {
        if(minr>tover_out[j*dims2[1]+i]){
          minr=tover_out[j*dims2[1]+i];
          mini=i;
          minj=j;
        }
      }
    }
    if(minr<100){
      for(int j=0;j<dims1[1];j++){
        tover_out[j*dims2[1]+mini]=100;
      }
      for (int i=0; i<dims2[1]; i++){
        tover_out[minj*dims2[1]+i]=100;
      }
      over_out[minj*dims2[1]+mini]=minr;
    }
  }while(minr<70);


  int dnbr=0;
  do{
    minr=1000000;
    for(int j=0;j<dims1[1];j++){
      for (int i=0; i<dims2[1]; i++){
        if(minr>tdesc_out[j*dims2[1]+i]){
          minr=tdesc_out[j*dims2[1]+i];
          mini=i;
          minj=j;
        }
      }
    }
    if(minr<1000000){
      for(int j=0;j<dims1[1];j++){
        tdesc_out[j*dims2[1]+mini]=1000000;
      }
      for (int i=0; i<dims2[1]; i++){
        tdesc_out[minj*dims2[1]+i]=1000000;
      }
      desc_out[minj*dims2[1]+mini]=dnbr++;//minr
    }
  }while(minr<1000000);

  delete []odim;
  delete []tdesc_out;
  delete []tover_out;
  delete []feat1a;
  delete []feat2a;
}

//Stupid dummy main function. Otherwise will tigger a linking error on MAC OS
int main()
{
  return 0;
}
