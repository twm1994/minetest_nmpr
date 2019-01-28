/*
modified on npc.cpp
*/

#include "npc.h"
#include "map.h"
#include "connection.h"

Npc::Npc(
	scene::ISceneNode* parent,
	scene::ISceneManager* mgr,
	s32 id) :
	scene::ISceneNode(parent, mgr, id),
	speed(0, 0, 0),
	touching_ground(false),
	npc_id(NPC_ID_NEW),
	m_position(0, 0, 0)
{
	m_box = core::aabbox3d<f32>(-BS, -BS, -BS, BS, BS, BS);

	video::IVideoDriver* driver = SceneManager->getVideoDriver();
	avatar = mgr->getMesh("../data/character.b3d");
	avatar_node = mgr->addAnimatedMeshSceneNode(avatar, this);
	if (avatar_node) {
		avatar_node->setMaterialFlag(video::EMF_LIGHTING, false);
		avatar_node->setMaterialTexture(0, driver->getTexture("../data/character.png"));
		avatar_node->setFrameLoop(200, 220);
		avatar_node->setAnimationSpeed(8);
		// this->animateStand();
		avatar_node->setScale(v3f(1, 1, 1));
		avatar_node->setPosition(v3f(0, 0, 0));
		avatar_node->setRotation(v3f(0, 0, 0));
	}
	updateSceneNodePosition();
	updateSceneNodeRotation();
}

Npc::~Npc()
{
	if (SceneManager != NULL)
		ISceneNode::remove();
}

// void Npc::stand()
// {
// 	avatar_node->setFrameLoop(0,80);
// }

// void Npc::sit()
// {
// 	avatar_node->setFrameLoop(81,161);
// }

void Npc::move(f32 dtime, Map &map)
{
	//v3f rotation = getRotation();
	//setRotation(rotation);
	
	// this->avatar_node->setFrameLoop(168,188);
	v3f position = getPosition();
	v3f oldpos = position;
	v3s16 oldpos_i = Map::floatToInt(oldpos);

	/*std::cout<<"oldpos_i=("<<oldpos_i.X<<","<<oldpos_i.Y<<","
			<<oldpos_i.Z<<")"<<std::endl;*/

	position += speed * dtime;

	v3s16 pos_i = Map::floatToInt(position);

	// The frame length is limited to the npc going 0.1*BS per call
	f32 d = (float)BS * 0.15;

#define NPC_RADIUS (BS*0.3)
#define NPC_HEIGHT (BS*1.7)

	core::aabbox3d<f32> npcbox(
		position.X - NPC_RADIUS,
		position.Y - 0.0,
		position.Z - NPC_RADIUS,
		position.X + NPC_RADIUS,
		position.Y + NPC_HEIGHT,
		position.Z + NPC_RADIUS
	);
	core::aabbox3d<f32> npcbox_old(
		oldpos.X - NPC_RADIUS,
		oldpos.Y - 0.0,
		oldpos.Z - NPC_RADIUS,
		oldpos.X + NPC_RADIUS,
		oldpos.Y + NPC_HEIGHT,
		oldpos.Z + NPC_RADIUS
	);

	//hilightboxes.push_back(npcbox);

	touching_ground = false;

	/*std::cout<<"Checking collisions for ("
			<<oldpos_i.X<<","<<oldpos_i.Y<<","<<oldpos_i.Z
			<<") -> ("
			<<pos_i.X<<","<<pos_i.Y<<","<<pos_i.Z
			<<"):"<<std::endl;*/

	for (s16 y = oldpos_i.Y - 1; y <= oldpos_i.Y + 2; y++) {
		for (s16 z = oldpos_i.Z - 1; z <= oldpos_i.Z + 1; z++) {
			for (s16 x = oldpos_i.X - 1; x <= oldpos_i.X + 1; x++) {
				//std::cout<<"with ("<<x<<","<<y<<","<<z<<"): ";
				try {
					if (map.getNode(x, y, z).d == MATERIAL_AIR) {
						//std::cout<<"air."<<std::endl;
						continue;
					}
				}
				catch (InvalidPositionException &e)
				{
					// Doing nothing here will block the npc from
					// walking over map borders
				}

				core::aabbox3d<f32> nodebox = Map::getNodeBox(
					v3s16(x, y, z));

				// See if the npc is touching ground
				if (
					/*(nodebox.MaxEdge.Y+d > npcbox.MinEdge.Y &&
					nodebox.MaxEdge.Y-d < npcbox.MinEdge.Y)*/
					fabs(nodebox.MaxEdge.Y - npcbox.MinEdge.Y) < d
					&& nodebox.MaxEdge.X - d > npcbox.MinEdge.X
					&& nodebox.MinEdge.X + d < npcbox.MaxEdge.X
					&& nodebox.MaxEdge.Z - d > npcbox.MinEdge.Z
					&& nodebox.MinEdge.Z + d < npcbox.MaxEdge.Z
					) {
					touching_ground = true;
				}

				if (npcbox.intersectsWithBox(nodebox))
				{

					v3f dirs[3] = {
						v3f(0,0,1), // back
						v3f(0,1,0), // top
						v3f(1,0,0), // right
					};
					for (u16 i = 0; i < 3; i++)
					{
						f32 nodemax = nodebox.MaxEdge.dotProduct(dirs[i]);
						f32 nodemin = nodebox.MinEdge.dotProduct(dirs[i]);
						f32 npcmax = npcbox.MaxEdge.dotProduct(dirs[i]);
						f32 npcmin = npcbox.MinEdge.dotProduct(dirs[i]);
						f32 npcmax_old = npcbox_old.MaxEdge.dotProduct(dirs[i]);
						f32 npcmin_old = npcbox_old.MinEdge.dotProduct(dirs[i]);

						bool main_edge_collides =
							((nodemax > npcmin && nodemax <= npcmin_old + d
								&& speed.dotProduct(dirs[i]) < 0)
								||
								(nodemin < npcmax && nodemin >= npcmax_old - d
									&& speed.dotProduct(dirs[i]) > 0));

						bool other_edges_collide = true;
						for (u16 j = 0; j < 3; j++)
						{
							if (j == i)
								continue;
							f32 nodemax = nodebox.MaxEdge.dotProduct(dirs[j]);
							f32 nodemin = nodebox.MinEdge.dotProduct(dirs[j]);
							f32 npcmax = npcbox.MaxEdge.dotProduct(dirs[j]);
							f32 npcmin = npcbox.MinEdge.dotProduct(dirs[j]);
							if (!(nodemax - d > npcmin && nodemin + d < npcmax))
							{
								other_edges_collide = false;
								break;
							}
						}

						if (main_edge_collides && other_edges_collide)
						{
							speed -= speed.dotProduct(dirs[i]) * dirs[i];
							position -= position.dotProduct(dirs[i]) * dirs[i];
							position += oldpos.dotProduct(dirs[i]) * dirs[i];
						}

					}
				} // if(npcbox.intersectsWithBox(nodebox))
			} // for x
		} // for z
	} // for y

	setPosition(position);
	// avatar_node->setFrameLoop(168,188);
	// this->avatar_node->setFrameLoop(0,80);
}

