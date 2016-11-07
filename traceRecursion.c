#include <stdio.h>
#include <stdlib.h>

#define MAX_DEPTH 7 // max depth for recursion

double* trace(Object objects[], double Ro, double Rd, int numObjects, int currDepth, double* createdColor)
{
    double* color = malloc(sizeof(double)*3);


    double best_t = INFINITY;
    //printf("%d", Rd);
    // Go through each of the objects at each pixel and find out if they will intersect
    int best_i = 0;
    for (i = 0; i < numObjects; i++)
    {
        double t = 0;
        if(strcmp(objects[i].type, "sphere") == 0){

            t = sphereIntersection(objects[i].properties.sphere.position, objects[i].properties.sphere.radius,Rd, Ro);
            //printf("This is i:%d", i);
        } else if(strcmp(objects[i].type, "plane") == 0){

            t = planeIntersection(objects[i].properties.plane.position, objects[i].properties.plane.normal, Rd, Ro);
            //printf("%d", i);
        }

        // If they do set the t as well as the object that is intersecting
        if (t > 0 && t < best_t)
        {
            best_t = t;
            best_i = i;
            //printf("%d", t);
        }
        int l,k;

        // place the color of the current intersection into the image buffer
        if(best_t > 0 && best_t != INFINITY && best_t != -1)
        {
            if(strcmp(objects[best_i].type, "sphere") == 0)
            {  // printf("hello there!");
                double reflection = objects[best_i].properties.sphere.reflectivity;
                double refraction = objects[best_i].properties.sphere.refractivity;
                double indexOfRefraction = objects[best_i].properties.sphere.ior;

                // if there is a refraction or reflection or ior then use them
                if(reflection >= 0 || refraction >= 0 || indexOfRefraction >=0 && currDepth < MAX_DEPTH)
                {
                        double temp[3];
                        double Ron[3];//phit
                        double Rdn[3];//nhit
                        v3_scale(Rd, best_t, temp);
                        v3_add(temp, Ro, Ron);
                        v3_subtract(Ron, objects[best_i].properties.sphere.position, Rdn);

                        normalize(Rdn);
                        bool inside = false;
                        double bias = 0.0001;
                        if (v3_dot(Rd,nhit) > 0)
                        {
                            v3_scale(Rdn, -1, Rdn)
                            inside = true;
                        }

                        double rayDir = {-Rd[0], -Rd[1], -Rd[2]}; // inverse of the ray direction
                        double facingratio = v3_dot(rayDir,Rdn);
                        double fresneleffect = freselCalc(pow(1 - facingratio, 3), 1, 0.1);
                        // Compute the reflection direction
                        double refldir = v3_reflect(Rd, Rdn);
                        normalize(refldir);
                        double reflection[3];
                        double nextRayO[3];
                        v3_add(Ron, Rdn, nextRayO);
                        v3_scale(nextRayD, bias, nextRayO);
                        trace(objects,nextRayO, refldir, numObjects, 1, reflection);

                        double refraction = {0,0,0};
                        //If the object for sure has refraction/transparent
                        if(refract > 0)
                        {
                            double eta = 0;
                            if(inside) eta = indexOfRefraction;
                            else eta = 1/indexOfRefraction;
                            double inRdn = {-Rdn[0], -Rdn[1], -Rdn[2]};
                            double cosi = v3_dot(inRdn, Rd);
                            double k = 1 - pow(eta, eta) * (1 - cosi * cosi);
                            double mathTemp = eta * cosi - sqrt(k);
                            double refractD[3];
                            double raydTemp[3];
                            v3_scale(Rdn, mathTemp, raydTemp);
                            v3_scale(Rd, eta, refractD);
                            v3_add(refractD, raydTemp, refractD);
                            double refractO[3];
                            v3_scale(Rdn, bias, refractO);
                            v3_subtract(Ron, refractO, refractO);


                            normalize(refractD);
                            trace(objects, refractO, refractD, numObjects, 1, refraction);


                            color[0] += reflection[0] * fresneleffect + refraction[0] * (1 - fresneleffect) * refract * objects[best_i].properties.sphere.diffuseColor[0];
                            color[1] += reflection[1] * fresneleffect + refraction[1] * (1 - fresneleffect) * refract * objects[best_i].properties.sphere.diffuseColor[1];;
                            color[2] += reflection[2] * fresneleffect + refraction[2] * (1 - fresneleffect) * refract * objects[best_i].properties.sphere.diffuseColor[2];;

                }
                // if there is no reflection, refraction, or ior
                else
                {
                    color[0] = 0;//ambientColor[0];
                    color[1] = 0;//ambientColor[1];
                    color[2] = 0;//ambientColor[2];
                    // In order to find a shadow
                    for (l = 0; l < numObjects; l++)
                    {
                        // Look for a light to see if that object has a shadow casted on it by a light
                        if(strcmp(objects[l].type, "light") == 0)
                        {   // calc new ray origin and direction
                            double temp[3];
                            double Ron[3];
                            double Rdn[3];
                            v3_scale(Rd, best_t, temp);
                            v3_add(temp, Ro, Ron);
                            v3_subtract(objects[l].properties.light.position, Ron, Rdn);

                            double distanceTLight = v3_len(Rdn);
                            normalize(Rdn);
                            //printf("%lf", distanceTLight);
                            //exit(0);
                            double otherObjIntersect = shadeCheck(objects, Rdn, Ron, numObjects, best_i, distanceTLight);
                            // there is an object in the way so shade in
                            if(otherObjIntersect != -1)
                            {   //printf("hi there");

                                continue;
                            }
                            // otherwise there is no object in between us and the light source
                            else
                            {
                                //printf("hello");
                                double spherePos[3] = {objects[best_i].properties.sphere.position[0],objects[best_i].properties.sphere.position[1],objects[best_i].properties.sphere.position[2]};

                                double n[3] = {Ron[0] - spherePos[0], Ron[1]-spherePos[1], Ron[2]-spherePos[2]}; //objects[best_i]->normal;
                                //n = v3_subtract(Ron, objects[best_i].properties.sphere.position, n);

                                normalize(n);
                                double lVector[3] = {Rdn[0], Rdn[1], Rdn[2]}; // light vector is just Rdn
                                normalize(lVector);
                                //double distanceVector[3] = {Ron[0] - objects[best_i].properties.light.position[0], Ron[1] - objects[best_i].properties.light.position[1],Ron[2] - objects[best_i].properties.light.position[2]};

                                double lReflection[3];
                                normalize(lReflection);
                                double V[3] = {Rd[0], Rd[1], Rd[2]}; // original ray direction
                                //normalize(V);
                                double diffuseColor[3];
                                double specularColor[3];
                                double diffPlusSpec[3];
                                double lightRayToClosestObj[3];
                                //printf("problem is here\n");
                                v3_reflect(lVector, n, lReflection); // in the book

                                //printf("%d", objects[best_i].properties.sphere.diffuseColor[0]);
                                calculateDiffuse(n, lVector, objects[l].properties.light.color, objects[best_i].properties.sphere.diffuseColor, diffuseColor);
                                calculateSpecular(20, lVector, lReflection, n, V, objects[best_i].properties.sphere.specularColor, objects[l].properties.light.color, specularColor);

                                v3_add(diffuseColor, specularColor, diffPlusSpec);

                                v3_scale(Rdn, -1, lightRayToClosestObj);

                                double fang = calculateAngularAt(objects, Ron, numObjects, l);
                                double frad = calculateRadialAt(objects[l].properties.light.radialAOne, objects[l].properties.light.radialATwo, objects[l].properties.light.radialAZero, distanceTLight);
                                //printf("Color is: %lf", diffuseColor[1]);
                                //printf("Color is: %lf", diffPlusSpec[2]);

                                // still need frad * rest
                                color[0] += frad * fang * diffPlusSpec[0];
                                color[1] += frad * fang * diffPlusSpec[1];
                                color[2] += frad * fang * diffPlusSpec[2];


                            }

                        }

                    }
                }
                //put stuff into buffer here
            }
            else if(strcmp(objects[best_i].type, "plane") == 0)
            {   //printf("plane color");
                //printf("hi there!");

                color[0] = 0;//ambientColor[0];
                color[1] = 0;//ambientColor[1];
                color[2] = 0;//ambientColor[2];
                // In order to find a shadow
                for (l = 0; l < numObjects; l++)
                {
                    // Look for a light to see if that object has a shadow casted on it by a light
                    if(strcmp(objects[l].type, "light") == 0)
                    {   // calc new ray origin and direction
                        //printf("hello");
                        double temp[3];
                        double Ron[3];
                        double Rdn[3];
                        v3_scale(Rd, best_t, temp);
                        v3_add(temp, Ro, Ron);
                        v3_subtract(objects[l].properties.light.position, Ron, Rdn);

                        double distanceTLight = v3_len(Rdn);
                        normalize(Rdn);

                        double otherObjIntersect = shadeCheck(objects, Rdn, Ron, numObjects, best_i, distanceTLight);
                        // there is an object in the way so shade in
                        if(otherObjIntersect != -1)
                        {
                            continue;
                        }
                        // otherwise there is no object in between us and the light source
                        else
                        {
                            //printf("hello");
                            //double planePos[3] = {objects[best_i].properties.plane.position[0],objects[best_i].properties.plane.position[1],objects[best_i].properties.plane.position[2]};

                            double n[3] = {objects[best_i].properties.plane.normal[0],objects[best_i].properties.plane.normal[1],objects[best_i].properties.plane.normal[2]}; //objects[best_i]->normal;
                            //n = v3_subtract(Ron, objects[best_i].properties.sphere.position, n);
                            normalize(n);
                            double lVector[3] = {Rdn[0], Rdn[1], Rdn[2]}; // light vector is just Rdn
                            normalize(lVector);
                            double lReflection[3];
                            double V[3] = {-Rd[0], -Rd[1], -Rd[2]};
                            v3_reflect(lVector, n, lReflection); // in the book
                            normalize(lReflection);
                            double diffuseColor[3];
                            double specularColor[3];
                            double diffPlusSpec[3];

                            double lightRayToClosestObj[3];
                            //printf("problem is here\n");

                            //printf("%d", objects[best_i].properties.sphere.diffuseColor[0]);
                            calculateDiffuse(n, lVector, objects[l].properties.light.color, objects[best_i].properties.plane.diffuseColor, diffuseColor);
                            calculateSpecular(20, lVector, V, n, lReflection, objects[best_i].properties.plane.specularColor, objects[l].properties.light.color, specularColor);

                            v3_add(diffuseColor, specularColor, diffPlusSpec);

                            v3_scale(Rdn, -1, lightRayToClosestObj);

                            double fang = calculateAngularAt(objects, Ron, numObjects, l);
                            double frad = calculateRadialAt(objects[l].properties.light.radialAOne, objects[l].properties.light.radialATwo, objects[l].properties.light.radialAZero, distanceTLight);

                            //printf("Color is: %lf", diffuseColor[1]);
                            //printf("Color is: %lf", diffPlusSpec[2]);

                            // still need frad * rest
                            color[0] += frad * fang * diffPlusSpec[0];
                            color[1] += frad * fang * diffPlusSpec[1];
                            color[2] += frad * fang * diffPlusSpec[2];


                        }

                    }

                }
               // here is where we would normally put stuff in the buffer
            }

        }
        // If no intersection let black be the background
        else
        { // printf("no intersection!!\n");

            color[0] = 0*255 ;
            color[1] = 0*255;
            color[2] = 0*255;

        }
    }
    //Return color here!
	return color;
}
