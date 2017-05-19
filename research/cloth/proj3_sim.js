/*
 * Global variables
 */
var meshResolution;

// Particle states
var mass;
var vertexPosition, vertexNormal;
var vertexVelocity;

// Spring properties
var K, restLength; 

// Force parameters
var Cd;
var uf, Cv;
var g = 9.8;


/*
 * Getters and setters
 */
function getPosition(i, j) {
    var id = i*meshResolution + j;
    return vec3.create([vertexPosition[3*id], vertexPosition[3*id + 1], vertexPosition[3*id + 2]]);
}

function setPosition(i, j, x) {
    var id = i*meshResolution + j;
    vertexPosition[3*id] = x[0]; vertexPosition[3*id + 1] = x[1]; vertexPosition[3*id + 2] = x[2];
}

function getNormal(i, j) {
    var id = i*meshResolution + j;
    return vec3.create([vertexNormal[3*id], vertexNormal[3*id + 1], vertexNormal[3*id + 2]]);
}

function getVelocity(i, j) {
    var id = i*meshResolution + j;
    return vec3.create(vertexVelocity[id]);
}

function setVelocity(i, j, v) {
    var id = i*meshResolution + j;
    vertexVelocity[id] = vec3.create(v);
}


/*
 * Provided global functions (you do NOT have to modify them)
 */
function computeNormals() {
    var dx = [1, 1, 0, -1, -1, 0], dy = [0, 1, 1, 0, -1, -1];
    var e1, e2;
    var i, j, k = 0, t;
    for ( i = 0; i < meshResolution; ++i )
        for ( j = 0; j < meshResolution; ++j ) {
            var p0 = getPosition(i, j), norms = [];
            for ( t = 0; t < 6; ++t ) {
                var i1 = i + dy[t], j1 = j + dx[t];
                var i2 = i + dy[(t + 1) % 6], j2 = j + dx[(t + 1) % 6];
                if ( i1 >= 0 && i1 < meshResolution && j1 >= 0 && j1 < meshResolution &&
                     i2 >= 0 && i2 < meshResolution && j2 >= 0 && j2 < meshResolution ) {
                    e1 = vec3.subtract(getPosition(i1, j1), p0);
                    e2 = vec3.subtract(getPosition(i2, j2), p0);
                    norms.push(vec3.normalize(vec3.cross(e1, e2)));
                }
            }
            e1 = vec3.create();
            for ( t = 0; t < norms.length; ++t ) vec3.add(e1, norms[t]);
            vec3.normalize(e1);
            vertexNormal[3*k] = e1[0];
            vertexNormal[3*k + 1] = e1[1];
            vertexNormal[3*k + 2] = e1[2];
            ++k;
        }
}


var clothIndex, clothWireIndex;
function initMesh() {
    var i, j, k;

    vertexPosition = new Array(meshResolution*meshResolution*3);
    vertexNormal = new Array(meshResolution*meshResolution*3);
    clothIndex = new Array((meshResolution - 1)*(meshResolution - 1)*6);
    clothWireIndex = [];

    vertexVelocity = new Array(meshResolution*meshResolution);
    restLength[0] = 4.0/(meshResolution - 1);
    restLength[1] = Math.sqrt(2.0)*4.0/(meshResolution - 1);
    restLength[2] = 2.0*restLength[0];

    for ( i = 0; i < meshResolution; ++i )
        for ( j = 0; j < meshResolution; ++j ) {
            setPosition(i, j, [-2.0 + 4.0*j/(meshResolution - 1), -2.0 + 4.0*i/(meshResolution - 1), 0.0]);
            setVelocity(i, j, vec3.create());

            if ( j < meshResolution - 1 )
                clothWireIndex.push(i*meshResolution + j, i*meshResolution + j + 1);
            if ( i < meshResolution - 1 )
                clothWireIndex.push(i*meshResolution + j, (i + 1)*meshResolution + j);
            if ( i < meshResolution - 1 && j < meshResolution - 1 )
                clothWireIndex.push(i*meshResolution + j, (i + 1)*meshResolution + j + 1);
        }
    computeNormals();

    k = 0;
    for ( i = 0; i < meshResolution - 1; ++i )
        for ( j = 0; j < meshResolution - 1; ++j ) {
            clothIndex[6*k] = i*meshResolution + j;
            clothIndex[6*k + 1] = i*meshResolution + j + 1;
            clothIndex[6*k + 2] = (i + 1)*meshResolution + j + 1;
            clothIndex[6*k + 3] = i*meshResolution + j;
            clothIndex[6*k + 4] = (i + 1)*meshResolution + j + 1;            
            clothIndex[6*k + 5] = (i + 1)*meshResolution + j;
            ++k;
        }
}


/*
 * KEY function: simulate one time-step using Euler's method
 */
function f_gravity()
{
    return vec3.create([0, -mass*g, 0]);
}

function f_spring(p, q, type)
{
    var L0 = restLength[type];
    var K0 = K[type];
    var d = [];
    vec3.subtract(p, q, d);
    var len = vec3.length(d);
    var f = [];
    return vec3.scale(d, K0*(L0 - len)/len, f);
}

function f_damp(v)
{
    var f = [];
    return vec3.scale(v, -Cd, f);
}

function f_viscous(n, v)
{
    var d = [];
    vec3.subtract(uf, v, d);
    var f = [];
    return vec3.scale(n, Cv*vec3.dot(d,n), f);
}

function simulate(stepSize) {      
    var temp_x = [];
    var temp_v = [];
    
    for (i = 0; i < meshResolution; ++i) {
        for (j = 0; j < meshResolution; ++j) {
            if ((i == meshResolution - 1 && j == 0) || (i == meshResolution - 1 && j == meshResolution - 1)) {
                temp_x[i + j*meshResolution] = getPosition(i,j);
                temp_v[i + j*meshResolution] = 0;
                continue;
            }
            
            var a = f_gravity();
            
            // Structural.
            if (i > 0) {
                vec3.add(a, f_spring(getPosition(i, j), getPosition(i - 1, j), 0));
            }
            if (i < meshResolution - 1) {
                vec3.add(a, f_spring(getPosition(i, j), getPosition(i + 1, j), 0));
            }
            if (j > 0) {
                vec3.add(a, f_spring(getPosition(i, j), getPosition(i, j - 1), 0));
            }
            if (j < meshResolution - 1) {
                vec3.add(a, f_spring(getPosition(i, j), getPosition(i, j + 1), 0));
            }
            
            // Shear
            if (i > 0 && j > 0) {
                vec3.add(a, f_spring(getPosition(i, j), getPosition(i - 1, j - 1), 1));
            }
            if (i < meshResolution - 1 && j < meshResolution - 1) {
                vec3.add(a, f_spring(getPosition(i, j), getPosition(i + 1, j + 1), 1));
            }
            if (i > 0 && j < meshResolution - 1) {
                vec3.add(a, f_spring(getPosition(i, j), getPosition(i - 1, j + 1), 1));
            }
            if (i < meshResolution - 1 && j > 0) {
                vec3.add(a, f_spring(getPosition(i, j), getPosition(i + 1, j - 1), 1));
            }
            
            // Flexion
            if (i > 1) {
                vec3.add(a, f_spring(getPosition(i, j), getPosition(i - 2, j), 2));
            }
            if (i < meshResolution - 2) {
                vec3.add(a, f_spring(getPosition(i, j), getPosition(i + 2, j), 2));
            }
            if (j > 1) {
                vec3.add(a, f_spring(getPosition(i, j), getPosition(i, j - 2), 2));
            }
            if (j < meshResolution - 2) {
                vec3.add(a, f_spring(getPosition(i, j), getPosition(i, j + 2), 2));
            }
            
            
            vec3.add(a, f_damp(getVelocity(i,j)));
            vec3.add(a, f_viscous(getNormal(i,j), getVelocity(i,j)));
            
            vec3.scale(a, 1.0/mass);
            
            var delta_v = [];
            vec3.scale(a, stepSize, delta_v)
            var vt_1 = [];
            vec3.add(getVelocity(i, j), delta_v, vt_1);
            
            var delta_x = [];
            vec3.scale(vt_1, stepSize, delta_x);
            var x = [];
            vec3.add(getPosition(i, j), delta_x, x);
            
            temp_x[i + j*meshResolution] = x;
            temp_v[i + j*meshResolution] = vt_1;
        }
    }
    
    for (i = 0; i < meshResolution; ++i) {
        for (j = 0; j < meshResolution; ++j) {
            setPosition(i, j, temp_x[i + j*meshResolution]);
            setVelocity(i, j, temp_v[i + j*meshResolution]);
        }
    }
}
