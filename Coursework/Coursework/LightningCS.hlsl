#define groupthreads 16


[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID, 
    uint3 Gid : SV_GroupID, 
    uint GI : SV_GroupIndex)
{

    GroupMemoryBarrierWithGroupSync();

}