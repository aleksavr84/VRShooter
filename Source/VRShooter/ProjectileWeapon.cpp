#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    APawn* InstigatorPawn = Cast<APawn>(GetOwner());
    const USkeletalMeshSocket* MuzzleFlashSocket = GetMuzzleFlashSocket();
    
    if (MuzzleFlashSocket)
    {   
        FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetItemMesh());

        // From muzzle flesh socket to hit location from TraceUnderCrosshairs.
        //FVector ToTarget = HitTarget - SocketTransform.GetLocation();
        FVector ToTarget = SocketTransform.GetLocation();
        FQuat Rotation = SocketTransform.GetRotation();
        
        FRotator TargetRotation = FRotator(Rotation);;//ToTarget.Rotation();

        if (GetProjectileClass() && InstigatorPawn)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = GetOwner();
            SpawnParams.Instigator = InstigatorPawn;
            UWorld* World = GetWorld();

            if (World)
            {
                World->SpawnActor<AProjectile>(
                    GetProjectileClass(),
                    SocketTransform.GetLocation(),
                    TargetRotation,
                    SpawnParams
                    );
            }
        }
    }
}
