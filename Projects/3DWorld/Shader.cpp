#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"


using namespace std;
using namespace glm;

// Compile |source| as a shader program of the indicated type.  Throw
// any compile errors as an exception.  Return shaderId on success.
GLuint Shader::CompileShader(const char *source, GLenum shdType) {
   int ok;
   constexpr int cMaxLogLen = 1000;
   char logBuf[cMaxLogLen + 1];

   // Create an empty vertex shader handle
   int shdId = glCreateShader(shdType);
    
   // Send the vertex shader source code to OpenGL
   glShaderSource(shdId, (GLsizei)1, &source, NULL);

   glCompileShader(shdId);

   // check shader for errors
   glGetShaderiv(shdId, GL_COMPILE_STATUS, &ok);
   if (!ok) {
      glGetShaderInfoLog(shdId, cMaxLogLen, NULL, logBuf);
      throw WorldException(logBuf);
   }

   return shdId;
}

// Link all shaderIds in |shaders| into a shader program, returning the new
// program's ID on success.  Throw any link errors as an exception.
GLuint Shader::LinkShaders(std::vector<GLuint> shds) {
   int ok;
   constexpr int cMaxLogLen = 1000;
   char logBuf[cMaxLogLen + 1];

   mProgramID = glCreateProgram();

   for (auto shdId: shds)
      glAttachShader(mProgramID, shdId);

   // combines all different shader programs
   glLinkProgram(mProgramID);

   // check for shader errors after linking
   glGetProgramiv(mProgramID, GL_LINK_STATUS, &ok);
   if (!ok) {
      glGetProgramInfoLog(mProgramID, cMaxLogLen, NULL, logBuf); GLChkErr;
      throw WorldException(logBuf);
   }

   glUseProgram(mProgramID);

   // set the active texture values for each texture
   glUniform1i(glGetUniformLocation(mProgramID, "tex"), 0);
   glUniform1i(glGetUniformLocation(mProgramID, "normalMap"), 1);
   glUniform1i(glGetUniformLocation(mProgramID, "shadowMap"), 2);

   return mProgramID;
}

Shader::Shader() {
   vector<GLuint> shaders;

   // Vertex shader
   const char * vertShader = R"(
#version 330

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 tex_Coord;
layout(location = 3) in vec3 in_Tan;
layout(location = 4) in vec3 in_BiTan;

uniform mat4 mvp;
uniform mat4 LSM;

out vec4 fragPos;
out vec3 fragNormal;
out vec2 fragTexCoord;
out vec3 fragVPos;
out vec4 fragLSM;
out mat3 TBN;

void main(void) {
   gl_Position = fragPos = mvp * in_Position;
  
   fragVPos = vec3(in_Position);
   fragNormal = in_Normal;
   fragTexCoord = tex_Coord;
   fragLSM = LSM * in_Position;
   TBN = transpose(mat3(
      normalize(in_Tan),
      normalize(in_BiTan),
      normalize(in_Normal)
   ));
}

)";

   // fragment shader
   const char * fragShader = R"(
#version 330

precision highp float;
 
struct Light 
{
   vec4 lPos;
   vec3 lColor;
};
 
uniform sampler2D tex;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;
uniform mat3 nvp;
uniform mat4 mvp;
uniform int numLights;
uniform Light lights[5];
uniform bool normMap;
uniform vec3 absPos;

in vec4 fragPos;
in vec4 fragLSM;
in vec3 fragNormal;
in vec3 fragVPos;
in vec2 fragTexCoord;
in mat3 TBN;

out vec4 fragColor;

float ShadowCalculation(vec4 PLS, vec3 lightPos)
{
   // perform perspective divide
   vec3 projCoords = PLS.xyz / PLS.w;

   // transform to [0,1] range
   projCoords = projCoords * 0.5 + 0.5;

   // get closest depth value from light's perspective 
   // (using [0,1] range fragPosLight as coords)
   float closestDepth = texture(shadowMap, projCoords.xy).r; 

   // get depth of current fragment from light's perspective
   float currentDepth = projCoords.z;

   // calculate bias (based on depth map resolution and slope)
   vec3 normal = normalize(fragNormal);
   vec3 lightDir = normalize(lightPos - vec3(fragPos));
   float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

   // check whether current frag pos is in shadow
   // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
   // PCF
   float shadow = 0.0;
   vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
   for(int x = -1; x <= 1; ++x)
   {
      for(int y = -1; y <= 1; ++y)
      {
         float pcfDepth = texture(
         shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
         shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
      }    
   }
   shadow /= 9.0;
    
   // keep the shadow at 0.0 when outside the 
   // far_plane region of the light's frustum.
   if(projCoords.z > 1.0)
      shadow = 0.0;
        
   return shadow;
}  

 
void main(void) {
   vec3 diffuse = vec3(0.0, 0.0, 0.0);
   vec3 ambient = vec3(0.0, 0.0, 0.0);
   vec3 specular = vec3(0.0, 0.0, 0.0);
   vec3 lighting = vec3(0, 0, 0);

   vec3 tempClr = texture(tex, fragTexCoord).rgb;
   ambient = tempClr * .2;

   for (int i = 0; i < numLights; i++) {
      if (normMap) {
         vec3 normal = texture(normalMap, fragTexCoord).rgb;
         normal =  normalize(normal * 2.0 - 1.0);

         vec3 lightDir = normalize(
          TBN * vec3(lights[i].lPos) - TBN * fragVPos);
         float diff = max(dot(lightDir, normal), 0.0);
         diffuse = diffuse + (diff * tempClr);

         vec3 viewDir = normalize(TBN * absPos - TBN * fragVPos);
         vec3 reflectDir = reflect(-lightDir, normal);
         vec3 halfwayDir = normalize(lightDir + viewDir);  
         float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
         specular = specular + (vec3(0.2) * spec);

         float shadow = ShadowCalculation(fragLSM, lights[i].lPos.xyz);       
         lighting = lighting +
          (ambient + (1.0 - shadow) * (diffuse + specular)) * tempClr; 
      }
      else {
         vec3 normal = normalize(nvp * fragNormal);
         vec3 lightVec = normalize(vec3(mvp * lights[i].lPos) - vec3(fragPos));

         float dfsBright = dot(normal, lightVec); 
	
         lighting = (dfsBright * lights[i].lColor) * tempClr;
      }
   }
   fragColor = vec4(lighting, 1);
}

)";

   // combine both files into single shader program
   shaders.push_back(CompileShader(fragShader, GL_FRAGMENT_SHADER));
   shaders.push_back(CompileShader(vertShader, GL_VERTEX_SHADER));

   mProgramID = LinkShaders(shaders);
}

/// set up shader lightsources and large ambient light
void Shader::Configure(vector<LightSource> lights, mat4 LSM) {

   glUseProgram(mProgramID);   GLChkErr;

   // passes Ambient light source
   glUniformMatrix4fv(glGetUniformLocation(mProgramID, "LSM"),
      1, GL_FALSE ,&(LSM)[0][0]); GLChkErr;

   glUniform1i(glGetUniformLocation(mProgramID, "numLights"), lights.size());
   GLChkErr;

   // passes loc and intesity of all point lights in scene
   for (int i = 0; i < lights.size(); i++) {
      glUniform4fv(
       glGetUniformLocation(mProgramID,
       StringPrintf("lights[%d].lPos", i).c_str()), 
       1, value_ptr(lights[i].location)); GLChkErr;

      glUniform3fv(
       glGetUniformLocation(mProgramID,
       StringPrintf("lights[%d].lColor", i).c_str()),
       1, value_ptr(lights[i].intensity)); GLChkErr;
   }
}

/// passes values for current render to shader
void Shader::Run(const glm::mat4x4 &xfm, vec3 absPos) {
   glUseProgram(mProgramID); GLChkErr;

   glUniform3fv(
    glGetUniformLocation(mProgramID, "absPos"), 1, &absPos[0]); GLChkErr;

   glUniformMatrix4fv(glGetUniformLocation(mProgramID, "mvp"), 1,
    GL_FALSE, &(xfm)[0][0]); GLChkErr;

   mat3 nvp = transpose(inverse(mat3(xfm)));
   glUniformMatrix3fv(glGetUniformLocation(mProgramID, "nvp"), 1, GL_FALSE,
    &nvp[0][0]); GLChkErr;
}

/// tells the shader if a normal map is in use.
void Shader::SetNMap(bool nMap) {
   glUniform1i(glGetUniformLocation(mProgramID, "normMap"), nMap); GLChkErr;
}

/// Sets up and creates single pass render shadow map
void Shader::SetShadowMap(mat4 lightSpaceMatrix) {
   vector<GLuint> shaders;

   // vertex shader
   const char * vertShader = R"(
#version 330
layout (location = 0) in vec4 aPos;

uniform mat4 lightSpaceMatrix;

void main() {
    gl_Position = lightSpaceMatrix * aPos;
}  

)";

   // fragment shader
   const char * fragShader = R"(
#version 330
void main() {             
}  
)";

   int ok;
   constexpr int cMaxLogLen = 1000;
   char logBuf[cMaxLogLen + 1];

   // Create an empty vertex shader handle
   int SSVertID = glCreateShader(GL_FRAGMENT_SHADER);

   // Send the vertex shader source code to OpenGL
   glShaderSource(SSVertID, (GLsizei)1, &fragShader, NULL);

   GL_TRUE;

   glCompileShader(SSVertID);
   glGetShaderiv(SSVertID, GL_COMPILE_STATUS, &ok);
   if (!ok) {
      glGetShaderInfoLog(SSVertID, cMaxLogLen, NULL, logBuf);
      throw WorldException(logBuf);
   }


   // Create an empty vertex shader handle
   int SSFragID = glCreateShader(GL_VERTEX_SHADER);

   // Send the vertex shader source code to OpenGL
   glShaderSource(SSFragID, (GLsizei)1, &vertShader, NULL);

   GL_TRUE;

   glCompileShader(SSFragID);
   glGetShaderiv(SSFragID, GL_COMPILE_STATUS, &ok);

   if (!ok) {
      glGetShaderInfoLog(SSFragID, cMaxLogLen, NULL, logBuf);
      throw WorldException(logBuf);
   }
   
   mShdwPID = glCreateProgram();

   glAttachShader(mShdwPID, SSVertID);
   glAttachShader(mShdwPID, SSFragID);

   glLinkProgram(mShdwPID);
   glGetProgramiv(mShdwPID, GL_LINK_STATUS, &ok);

   if (!ok) {
      glGetProgramInfoLog(mShdwPID, cMaxLogLen, NULL, logBuf); GLChkErr;
      throw WorldException(logBuf);
   }

   glUseProgram(mShdwPID);

   // pass in transformation matrix
   glUniformMatrix4fv(glGetUniformLocation(mProgramID, "lightSpaceMatrix"),
    1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
}