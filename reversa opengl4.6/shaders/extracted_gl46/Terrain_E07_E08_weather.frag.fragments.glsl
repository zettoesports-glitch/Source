// EXTRACTED printable GLSL fragments from Main.exe.
// File offsets: 0x00792EF9..0x00797388
// No invented statements; NUL-separated literals were joined with newlines.
in vec2 textureCoordinate;
in vec4 vertexColor;
in float viewDepth;
#if TERRAIN_WEATHER_FX
in vec3 worldPosition;
#endif
uniform sampler2D textureSampler;
uniform int useTexture;
uniform int alphaCutoffEnabled;
uniform float alphaReference;
uniform int fogEnabled;
#if TERRAIN_WEATHER_FX
uniform int terrainSurface;
uniform int weatherEnabled;
uniform int waterFxEnabled;
uniform int rainSurfaceFxEnabled;
uniform int wetGroundFxEnabled;
uniform int footstepFxEnabled;
uniform int weatherQuality;
uniform float weatherTime;
uniform float rainIntensity;
uniform float waterLevel;
uniform float waterDepthRange;
uniform float waterTurbidity;
uniform float waterFrost;
uniform float waterLava;
const int FOOTSTEP_MAX = 8;
uniform int footstepCount;
// Dois vec3 por passo: centro/idade e direcao/lado esquerdo-direito.
uniform vec3 footsteps[FOOTSTEP_MAX * 2];
#endif
layout(std140, binding = 0) uniform FrameData
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec4 cameraPosition;
    vec4 viewport;
    vec4 timeFog;
    vec4 tint;
};
layout(location = 0) out vec4 outputColor;
#if TERRAIN_WEATHER_FX
const vec3 WATER_SHALLOW = vec3(0.22, 0.62, 0.64);
const vec3 WATER_DEEP = vec3(0.015, 0.12, 0.20);
const vec3 WATER_SKY = vec3(0.42, 0.64, 0.78);
const vec3 WATER_FROST = vec3(0.60, 0.80, 0.86);
const vec3 WATER_FROST_DEEP = vec3(0.07, 0.20, 0.30);
const vec3 SUN_TINT = vec3(1.00, 0.97, 0.90);
const vec3 SUN_DIR = vec3(0.45, 0.42, 0.79);
const int WAVE_COUNT = 7;
const vec4 WAVES[WAVE_COUNT] = vec4[WAVE_COUNT](
    vec4( 0.92,  0.39, 0.021, 1.7),
    vec4(-0.51,  0.86, 0.033, 2.3),
    vec4( 0.30, -0.95, 0.058, 3.1),
    vec4( 0.80,  0.60, 0.095, 4.3),
    vec4(-0.97,  0.24, 0.171, 5.6),
    vec4( 0.14,  0.99, 0.288, 7.1),
    vec4(-0.66, -0.75, 0.463, 9.4));
const float WAVE_AMPS[WAVE_COUNT] = float[WAVE_COUNT](
    1.000, 0.620, 0.360, 0.200, 0.115, 0.068, 0.042);
const float RIPPLE_CELL = 85.0;
const float RIPPLE_MAX_R = 52.0;
const float RIPPLE_RATE = 0.85;
int activeWaveCount()
    return weatherQuality <= 0 ? 3 : (weatherQuality == 1 ? 5 : 7);
float waveHeight(vec2 p, float t, float detail)
    float height = 0.0;
    int count = activeWaveCount();
    for (int i = 0; i < WAVE_COUNT; ++i)
    {
        if (i >= count) break;
        float amplitude = WAVE_AMPS[i] * (i >= 4 ? detail : 1.0);
        height += sin(dot(p, WAVES[i].xy) * WAVES[i].z + t * WAVES[i].w)
            * amplitude;
    }
    return height;
vec2 waveGradient(vec2 p, float t, float detail)
    vec2 gradient = vec2(0.0);
    int count = activeWaveCount();
    for (int i = 0; i < WAVE_COUNT; ++i)
    {
        if (i >= count) break;
        float amplitude = WAVE_AMPS[i] * (i >= 4 ? detail : 1.0);
        gradient += cos(dot(p, WAVES[i].xy) * WAVES[i].z + t * WAVES[i].w)
            * amplitude * WAVES[i].z * WAVES[i].xy;
    }
    return gradient;
float hash21(vec2 p)
    p = fract(p * vec2(127.31, 311.7));
    p += dot(p, p + 34.53);
    return fract(p.x * p.y);
bool useRippleNeighbour(int x, int y)
    if (weatherQuality <= 0) return x == 0 && y == 0;
    if (weatherQuality == 1) return abs(x) + abs(y) <= 1;
    return true;
vec2 rainRipples(vec2 p, float t, float intensity, out float crest)
    crest = 0.0;
    vec2 gradient = vec2(0.0);
    if (intensity <= 0.001) return gradient;
    vec2 baseCell = floor(p / RIPPLE_CELL);
    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            if (!useRippleNeighbour(x, y)) continue;
            vec2 cell = baseCell + vec2(float(x), float(y));
            float random1 = hash21(cell);
            float random2 = hash21(cell + 37.19);
            if (random1 > intensity) continue;
            vec2 drop = (cell + vec2(random2, fract(random1 * 7.3))) * RIPPLE_CELL;
            float age = fract(t * RIPPLE_RATE + random2);
            vec2 delta = p - drop;
            float distanceToDrop = length(delta);
            float wave = distanceToDrop - age * RIPPLE_MAX_R;
            float band = max(0.0, 1.0 - abs(wave) * 0.10);
            float envelope = band * band * (1.0 - age);
            gradient += (delta / max(distanceToDrop, 0.001))
                * cos(wave * 0.55) * envelope;
            crest += cos(wave * 0.55) * envelope;
        }
    }
    return gradient;
float footprintEllipse(vec2 localPosition, vec2 center, vec2 radius)
    float distanceToSole = length((localPosition - center) / radius);
    return 1.0 - smoothstep(0.72, 1.0, distanceToSole);
float footprintBorder(vec2 localPosition, vec2 center, vec2 radius)
    float distanceToSole = length((localPosition - center) / radius);
    float outer = 1.0 - smoothstep(0.88, 1.05, distanceToSole);
    float inner = 1.0 - smoothstep(0.58, 0.78, distanceToSole);
    return max(outer - inner, 0.0);
vec2 footstepMark(vec2 p, out float wetMark, out float contactShine)
    wetMark = 0.0;
    contactShine = 0.0;
    vec2 gradient = vec2(0.0);
    int qualityLimit = weatherQuality <= 0 ? 2 : (weatherQuality == 1 ? 4 : 8);
    int count = min(footstepCount, qualityLimit);
    for (int i = 0; i < FOOTSTEP_MAX; ++i)
    {
        if (i >= count) break;
        vec3 stepData = footsteps[i * 2];
        vec3 stepDirection = footsteps[i * 2 + 1];
        vec2 direction = normalize(stepDirection.xy);
        vec2 lateral = vec2(-direction.y, direction.x);
        direction = normalize(direction + lateral * stepDirection.z * 0.10);
        lateral = vec2(-direction.y, direction.x);
        vec2 delta = p - stepData.xy;
        vec2 localPosition = vec2(dot(delta, direction), dot(delta, lateral));
        float age = stepData.z;
        float fade = pow(max(1.0 - age, 0.0), 1.45);
        float toe = footprintEllipse(
            localPosition, vec2(7.0, 0.0), vec2(16.0, 8.0));
        float middle = footprintEllipse(
            localPosition, vec2(-1.0, 0.0), vec2(12.5, 5.4));
        float heel = footprintEllipse(
            localPosition, vec2(-10.0, 0.0), vec2(10.0, 6.6));
        float sole = max(toe, max(middle * 0.88, heel * 0.96));
        wetMark += sole * fade;
        float toeBorder = footprintBorder(
            localPosition, vec2(7.0, 0.0), vec2(16.0, 8.0));
        float middleBorder = footprintBorder(
            localPosition, vec2(-1.0, 0.0), vec2(12.5, 5.4));
        float heelBorder = footprintBorder(
            localPosition, vec2(-10.0, 0.0), vec2(10.0, 6.6));
        float shineFade = pow(max(1.0 - age, 0.0), 0.85);
        float border = max(toeBorder,
            max(middleBorder * 0.80, heelBorder));
        contactShine += (border + sole * 0.13) * shineFade;
        gradient += direction * (toe - heel) * fade * 0.080;
    }
    return gradient;
vec3 shadeLava(vec4 textureColor)
    vec2 p = worldPosition.xy;
    vec3 toEye = cameraPosition.xyz - worldPosition;
    float distanceToEye = length(toEye);
    float detail = clamp(1.0 - (distanceToEye - 700.0) / 1500.0, 0.0, 1.0);
    vec2 drift = vec2(weatherTime * 1.7, weatherTime * -0.9);
    float crust = waveHeight((p + drift) * 0.030, weatherTime * 0.06, detail) * 0.5 + 0.5;
    float vein = waveHeight((p - drift * 0.6) * 0.075, weatherTime * 0.11, detail) * 0.5 + 0.5;
    float heat = clamp(dot(textureColor.rgb, vec3(0.55, 0.32, 0.13)) * 1.7, 0.0, 1.0);
    heat = clamp(heat + (1.0 - crust) * 0.50 + (1.0 - vein) * 0.22 - 0.24, 0.0, 1.0);
    const vec3 ROCK = vec3(0.052, 0.026, 0.020);
    const vec3 WARM = vec3(0.55, 0.11, 0.02);
    const vec3 HOT = vec3(1.00, 0.42, 0.06);
    const vec3 CORE = vec3(1.00, 0.86, 0.45);
    vec3 result = mix(ROCK, WARM, smoothstep(0.10, 0.45, heat));
    result = mix(result, HOT, smoothstep(0.45, 0.76, heat));
    result = mix(result, CORE, smoothstep(0.87, 1.00, heat));
    float pulse = sin(weatherTime * 0.55 + p.x * 0.004 + p.y * 0.003) * 0.5 + 0.5;
    result *= 1.0 + heat * pulse * 0.20;
    float seam = smoothstep(0.60, 0.88, 1.0 - crust) * detail;
    result += HOT * seam * heat * 0.32;
    return result;
vec3 shadeWater(vec4 textureColor)
    vec2 p = worldPosition.xy;
    vec3 toEye = cameraPosition.xyz - worldPosition;
    float distanceToEye = length(toEye);
    vec3 viewDirection = toEye / max(distanceToEye, 0.001);
    float detail = clamp(1.0 - (distanceToEye - 700.0) / 1500.0, 0.0, 1.0);
    float turbid = clamp(waterTurbidity, 0.0, 1.0);
    float frost = clamp(waterFrost, 0.0, 1.0);
    float thick = max(turbid, frost * 0.9);
    vec2 gradient = waveGradient(p * (1.0 + thick * 0.55),
        weatherTime * (1.0 - thick * 0.45) * (1.0 - frost * 0.45), detail)
        * (22.0 * (1.0 - thick * 0.55));
    float crest = 0.0;
    if (rainSurfaceFxEnabled != 0 && rainIntensity > 0.001)
    {
        gradient += rainRipples(p, weatherTime, rainIntensity, crest)
            * 4.5 * detail;
    }
    vec3 normal = normalize(vec3(-gradient.x, -gradient.y, 1.0));
    vec3 halfDirection = normalize(normalize(SUN_DIR) + viewDirection);
    float normalHalf = max(dot(normal, halfDirection), 0.0);
    float normalView = clamp(dot(normal, viewDirection), 0.0, 1.0);
    float sparkle = pow(normalHalf, 420.0) * 1.35 * detail;
    float sheen = pow(normalHalf, 30.0) * 0.14;
    float fresnel = 0.02 + 0.98 * pow(1.0 - normalView, 5.0);
    float depth = clamp((waterLevel - worldPosition.z)
        / max(waterDepthRange, 1.0), 0.0, 1.0);
    depth = pow(depth, 0.7);
    float textureLuminance = dot(textureColor.rgb, vec3(0.299, 0.587, 0.114));
    vec3 clearBody = mix(WATER_SHALLOW, WATER_DEEP, depth)
        * (0.68 + 0.48 * textureLuminance);
    vec3 murkyBody = textureColor.rgb * mix(1.02, 0.72, depth);
    vec3 result = mix(clearBody, murkyBody, turbid);
    if (turbid > 0.001)
    {
        float scum = waveHeight(p * 0.09, weatherTime * 0.10, detail) * 0.5 + 0.5;
        scum = smoothstep(0.35, 0.85, scum);
        result *= 1.0 - turbid * scum * 0.22;
        result = mix(result, result * vec3(0.86, 1.04, 0.82), turbid * scum * 0.5);
    }
    if (frost > 0.001)
    {
        vec3 chilled = mix(result, WATER_FROST, 0.55);
        chilled = mix(chilled, WATER_FROST_DEEP, depth * 0.65);
        result = mix(result, chilled, frost);
    }
    result = mix(result, WATER_SKY,
        fresnel * 0.48 * (1.0 - turbid * 0.88) * (1.0 + frost * 0.55));
    result += SUN_TINT * sparkle * (1.0 - turbid * 0.80) * (1.0 + frost * 1.10)
        + vec3(sheen) * (1.0 - turbid * 0.55) * (1.0 + frost * 0.85);
    float ringLight = max(crest, 0.0) * rainIntensity * detail;
    float ringShade = max(-crest, 0.0) * rainIntensity * detail;
    result *= 1.0 - ringShade * 0.16;
    result += vec3(0.42, 0.55, 0.62) * ringLight * 0.34;
    float shore = smoothstep(0.015, 0.045, depth)
        * (1.0 - smoothstep(0.10, 0.22, depth));
    float churn = waveHeight(p * 2.4, weatherTime * 1.6, detail) * 0.16;
    float foam = smoothstep(0.58, 0.96, shore + churn * 0.55);
    vec3 foamColor = mix(vec3(0.91, 0.96, 0.98),
        result * vec3(0.72, 0.86, 0.62), turbid);
    result = mix(result, foamColor, foam * (0.48 - turbid * 0.18));
    if (frost > 0.001)
    {
        float plate = waveHeight(p * 0.055, 0.0, detail) * 0.5 + 0.5;
        float crust = smoothstep(0.52, 0.60, plate);
        float rim = smoothstep(0.48, 0.52, plate) - smoothstep(0.60, 0.66, plate);
        vec3 ice = mix(vec3(0.78, 0.88, 0.92), vec3(0.90, 0.96, 0.99), crust);
        result = mix(result, ice, crust * frost * 0.62);
        result += vec3(0.30, 0.42, 0.48) * rim * frost * 0.35;
    }
    return result;
vec3 shadeWetGround(vec3 albedo)
    float rain = rainIntensity;
    if (rain <= 0.001) return albedo;
    vec3 toEye = cameraPosition.xyz - worldPosition;
    float distanceToEye = length(toEye);
    float detail = clamp(1.0 - (distanceToEye - 700.0) / 1500.0, 0.0, 1.0);
    vec3 wet = wetGroundFxEnabled != 0
        ? albedo * mix(1.0, 0.68, rain * 0.9) : albedo;
    if (detail <= 0.0) return wet;
    vec2 ripple = vec2(0.0);
    float crest = 0.0;
    if (rainSurfaceFxEnabled != 0)
    {
        ripple = rainRipples(worldPosition.xy, weatherTime, rain, crest) * detail;
    }
    float wetMark = 0.0;
    float contactShine = 0.0;
    if (footstepFxEnabled != 0 && footstepCount > 0)
    {
        ripple += footstepMark(worldPosition.xy, wetMark, contactShine)
            * 3.0 * detail;
        wetMark = min(wetMark, 1.0) * detail;
        contactShine = min(contactShine, 1.0) * detail;
    }
    vec3 normal = normalize(vec3(-ripple.x * 6.0, -ripple.y * 6.0, 1.0));
    vec3 viewDirection = toEye / max(distanceToEye, 0.001);
    vec3 halfDirection = normalize(normalize(SUN_DIR) + viewDirection);
    float specular = pow(max(dot(normal, halfDirection), 0.0), 90.0)
        * 0.50 * rain * detail;
    wet *= mix(1.0, 0.48, wetMark);
    specular += pow(max(dot(normal, halfDirection), 0.0), 140.0)
        * 0.28 * wetMark;
    wet += vec3(0.48, 0.57, 0.62) * contactShine * 0.52;
    wet += vec3(specular) + vec3(crest * 0.028 * rain * detail);
    return wet;
#endif
void main()
    vec4 color = clamp(vertexColor, 0.0, 1.0);
    if (useTexture != 0)
    {
        vec4 textureColor = texture(textureSampler, textureCoordinate);
#if TERRAIN_WEATHER_FX
        if (weatherEnabled != 0 && terrainSurface == 2 && waterFxEnabled != 0)
        {
            if (waterLava > 0.5)
            {
                vec3 lava = shadeLava(textureColor);
                color = vec4(mix(color.rgb * lava, lava, 0.70),
                    color.a * textureColor.a);
            }
            else
            {
                color = vec4(color.rgb * shadeWater(textureColor),
                    color.a * textureColor.a);
            }
        }
        else if (weatherEnabled != 0 && terrainSurface == 1
            && rainIntensity > 0.001
            && (wetGroundFxEnabled != 0 || rainSurfaceFxEnabled != 0
                || footstepFxEnabled != 0))
        {
            color = vec4(color.rgb * shadeWetGround(textureColor.rgb),
                color.a * textureColor.a);
        }
        else
        {
            color *= textureColor;
        }
#else
        color *= textureColor;
#endif
    }
    if (alphaCutoffEnabled != 0 && color.a <= alphaReference)
    {
        discard;
    }
    if (fogEnabled != 0 && timeFog.w > 0.0)
    {
        float fogRange = max(timeFog.z - timeFog.y, 0.0001);
        float fogFactor = clamp((viewDepth - timeFog.y) / fogRange, 0.0, 1.0);
        color.rgb = mix(color.rgb, tint.rgb, fogFactor * clamp(timeFog.w, 0.0, 1.0));
    }
    outputColor = color;
