#include <src/servers/Server_Common/Util/Util.h>
#include <src/servers/Server_Common/Exd/ExdData.h>
#include <src/servers/Server_Common/Util/UtilMath.h>

#include "ActionCollision.h"
#include <src/servers/Server_Zone/Actor/Actor.h>
#include <src/servers/Server_Zone/Actor/Player.h>
#include <cmath>
#include <boost/make_shared.hpp>

using namespace Core::Entity;
using namespace Core::Common;

// todo: add AoE actor limits (16, 32)

bool ActionCollision::isActorApplicable( ActorPtr actorPtr, TargetFilter targetFilter )
{
   bool actorApplicable = false;
   switch ( targetFilter )
   {
   case TargetFilter::All:
   {
      actorApplicable = true;
      break;
   }
   case TargetFilter::Players:
   {
      actorApplicable = actorPtr->isPlayer();
      break;
   }
   case TargetFilter::Allies:
   {
      // todo: implement ally NPCs
      actorApplicable = !actorPtr->isMob();
      break;
   }
   case TargetFilter::Party:
   {
      // todo: implement party
      actorApplicable = actorPtr->isPlayer();
      break;
   }
   case TargetFilter::Enemies:
   {
      actorApplicable = actorPtr->isMob();
      break;
   }
   }

   return ( actorApplicable && actorPtr->isAlive() );
}

std::set< Core::Entity::ActorPtr > ActionCollision::getActorsHitFromAction( FFXIVARR_POSITION3 aoePosition, std::set< ActorPtr > actorsInRange, boost::shared_ptr< Core::Data::ActionInfo > actionInfo, TargetFilter targetFilter )
{
   std::set< ActorPtr > actorsCollided;

   switch ( static_cast< ActionCollisionType >( actionInfo->aoe_type ) )
   {
   case ActionCollisionType::None:
   case ActionCollisionType::SingleTarget:
   {
      // This is actually needed. There is "splash damage" in actions marked as single target.
      // Notice how we're using aoe_width. How collision works for SingleTarget is unknown as of now.
      for ( auto pActor : actorsInRange )
      {
         // Make sure actor exists. If it doesn't we done goofed.
         assert( pActor );

         // Don't bother wasting on collision if actor doesn't apply for it
         if ( !isActorApplicable( pActor, targetFilter ) )
            continue;

         // Test our collision from actor with the area generated by the action from the AoE data
         if ( radiusCollision( pActor->getPos(), aoePosition, actionInfo->aoe_width ) )
         {
            // Add it to the actors collided with the area
            actorsCollided.insert( pActor );
         }
      }
      break;
   }
   case ActionCollisionType::Circle:
   {
      for ( auto pActor : actorsInRange )
      {
         assert( pActor );

         if ( !isActorApplicable( pActor, targetFilter ) )
            continue;

         if ( radiusCollision( pActor->getPos(), aoePosition, actionInfo->aoe_range ) )
         {
            actorsCollided.insert( pActor );
         }
      }
      break;
   }
   case ActionCollisionType::Box:
   {
      for ( auto pActor : actorsInRange )
      {
         assert( pActor );

         if ( !isActorApplicable( pActor, targetFilter ) )
            continue;

         if ( boxCollision( pActor->getPos(), aoePosition, actionInfo->aoe_width, actionInfo->aoe_range ) )
         {
            // todo: does this actually work?

            actorsCollided.insert( pActor );
         }
      }
      break;
   }
   default:
   {
      break;
   }
   }

   return actorsCollided;
}

bool ActionCollision::radiusCollision( FFXIVARR_POSITION3 actorPosition, FFXIVARR_POSITION3 aoePosition, uint16_t radius )
{
   return Core::Math::Util::distance( actorPosition.x, actorPosition.y, actorPosition.z,
      aoePosition.x, aoePosition.y, aoePosition.z ) <= radius;
}

bool ActionCollision::boxCollision( FFXIVARR_POSITION3 actorPosition, FFXIVARR_POSITION3 aoePosition, uint16_t width, uint16_t height )
{
   return actorPosition.x < aoePosition.x + width &&
      actorPosition.x > aoePosition.x &&
      actorPosition.y < aoePosition.y + height &&
      actorPosition.y > aoePosition.y;
}