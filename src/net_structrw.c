//
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// Reading and writing various structures into packets
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "i_system.h"
#include "m_misc.h"
#include "net_packet.h"
#include "net_structrw.h"

// String names for the enum values in net_protocol_t, which are what is
// sent over the wire. Every enum value must have an entry in this list.
static struct
{
    net_protocol_t protocol;
    const char *name;
} protocol_names[] = {
    {NET_PROTOCOL_CHOCOLATE_DOOM_0, "CHOCOLATE_DOOM_0"},
};

void NET_WriteConnectData(net_packet_t *packet, net_connect_data_t *data)
{
    NET_WriteInt8(packet, data->gamemode);
    NET_WriteInt8(packet, data->gamemission);
    NET_WriteInt8(packet, data->lowres_turn);
    NET_WriteInt8(packet, data->drone);
    NET_WriteInt8(packet, data->max_players);
    NET_WriteInt8(packet, data->is_freedoom);
    NET_WriteSHA1Sum(packet, data->wad_sha1sum);
    NET_WriteSHA1Sum(packet, data->deh_sha1sum);
    NET_WriteInt8(packet, data->player_class);
}

boolean NET_ReadConnectData(net_packet_t *packet, net_connect_data_t *data)
{
    return NET_ReadInt8(packet, (unsigned int *) &data->gamemode)
        && NET_ReadInt8(packet, (unsigned int *) &data->gamemission)
        && NET_ReadInt8(packet, (unsigned int *) &data->lowres_turn)
        && NET_ReadInt8(packet, (unsigned int *) &data->drone)
        && NET_ReadInt8(packet, (unsigned int *) &data->max_players)
        && NET_ReadInt8(packet, (unsigned int *) &data->is_freedoom)
        && NET_ReadSHA1Sum(packet, data->wad_sha1sum)
        && NET_ReadSHA1Sum(packet, data->deh_sha1sum)
        && NET_ReadInt8(packet, (unsigned int *) &data->player_class);
}

void NET_WriteSettings(net_packet_t *packet, net_gamesettings_t *settings)
{
    int i;

    NET_WriteInt8(packet, settings->ticdup);
    NET_WriteInt8(packet, settings->extratics);
    NET_WriteInt8(packet, settings->deathmatch);
    NET_WriteInt8(packet, settings->nomonsters);
    NET_WriteInt8(packet, settings->fast_monsters);
    NET_WriteInt8(packet, settings->respawn_monsters);
    NET_WriteInt8(packet, settings->episode);
    NET_WriteInt8(packet, settings->map);
    NET_WriteInt8(packet, settings->skill);
    NET_WriteInt8(packet, settings->gameversion);
    NET_WriteInt8(packet, settings->lowres_turn);
    NET_WriteInt8(packet, settings->new_sync);
    NET_WriteInt32(packet, settings->timelimit);
    NET_WriteInt8(packet, settings->loadgame);
    NET_WriteInt8(packet, settings->random);
    NET_WriteInt8(packet, settings->num_players);
    NET_WriteInt8(packet, settings->consoleplayer);

    // [Crispy Multiplayer Doom] Support -dropweapons.
    NET_WriteInt8(packet, settings->drop_weapons);
    // [Crispy Multiplayer Doom] Support -noplayercollisions.
    NET_WriteInt8(packet, settings->no_player_collisions);

    for (i = 0; i < settings->num_players; ++i)
    {
        NET_WriteInt8(packet, settings->player_classes[i]);
    }
}

boolean NET_ReadSettings(net_packet_t *packet, net_gamesettings_t *settings)
{
    boolean success;
    int i;

    success = NET_ReadInt8(packet, (unsigned int *) &settings->ticdup)
           && NET_ReadInt8(packet, (unsigned int *) &settings->extratics)
           && NET_ReadInt8(packet, (unsigned int *) &settings->deathmatch)
           && NET_ReadInt8(packet, (unsigned int *) &settings->nomonsters)
           && NET_ReadInt8(packet, (unsigned int *) &settings->fast_monsters)
           && NET_ReadInt8(packet, (unsigned int *) &settings->respawn_monsters)
           && NET_ReadInt8(packet, (unsigned int *) &settings->episode)
           && NET_ReadInt8(packet, (unsigned int *) &settings->map)
           && NET_ReadSInt8(packet, &settings->skill)
           && NET_ReadInt8(packet, (unsigned int *) &settings->gameversion)
           && NET_ReadInt8(packet, (unsigned int *) &settings->lowres_turn)
           && NET_ReadInt8(packet, (unsigned int *) &settings->new_sync)
           && NET_ReadInt32(packet, (unsigned int *) &settings->timelimit)
           && NET_ReadSInt8(packet, (signed int *) &settings->loadgame)
           && NET_ReadInt8(packet, (unsigned int *) &settings->random)
           && NET_ReadInt8(packet, (unsigned int *) &settings->num_players)
           && NET_ReadSInt8(packet, (signed int *) &settings->consoleplayer)
           // [Crispy Multiplayer Doom] Support -dropweapons.
           && NET_ReadInt8(packet, (unsigned int *) &settings->drop_weapons)
           // [Crispy Multiplayer Doom] Support -noplayercollisions.
           && NET_ReadInt8(packet, (unsigned int *) &settings->no_player_collisions);

    if (!success)
    {
        return false;
    }

    for (i = 0; i < settings->num_players; ++i)
    {
        if (!NET_ReadInt8(packet,
                          (unsigned int *) &settings->player_classes[i]))
        {
            return false;
        }
    }

    return true;
}

boolean NET_ReadQueryData(net_packet_t *packet, net_querydata_t *query)
{
    boolean success;

    query->version = NET_ReadSafeString(packet);

    success = query->version != NULL
          && NET_ReadInt8(packet, (unsigned int *) &query->server_state)
          && NET_ReadInt8(packet, (unsigned int *) &query->num_players)
          && NET_ReadInt8(packet, (unsigned int *) &query->max_players)
          && NET_ReadInt8(packet, (unsigned int *) &query->gamemode)
          && NET_ReadInt8(packet, (unsigned int *) &query->gamemission);

    if (!success)
    {
        return false;
    }

    query->description = NET_ReadSafeString(packet);

    // We read the list of protocols supported by the server. However,
    // old versions of Chocolate Doom do not support this field; it is
    // okay if it cannot be successfully read.
    query->protocol = NET_ReadProtocolList(packet);

    return query->description != NULL;
}

void NET_WriteQueryData(net_packet_t *packet, net_querydata_t *query)
{
    NET_WriteString(packet, query->version);
    NET_WriteInt8(packet, query->server_state);
    NET_WriteInt8(packet, query->num_players);
    NET_WriteInt8(packet, query->max_players);
    NET_WriteInt8(packet, query->gamemode);
    NET_WriteInt8(packet, query->gamemission);
    NET_WriteString(packet, query->description);

    // Write a list of all supported protocols. Note that the query->protocol
    // field is ignored here; it is only used when receiving.
    NET_WriteProtocolList(packet);
}

void NET_WriteTiccmdDiff(net_packet_t *packet, net_ticdiff_t *diff,
                         boolean lowres_turn)
{
    // Header

    NET_WriteInt8(packet, diff->diff);

    // Write the fields which are enabled:

    if (diff->diff & NET_TICDIFF_FORWARD)
        NET_WriteInt8(packet, diff->cmd.forwardmove);
    if (diff->diff & NET_TICDIFF_SIDE)
        NET_WriteInt8(packet, diff->cmd.sidemove);
    if (diff->diff & NET_TICDIFF_TURN)
    {
        if (lowres_turn)
        {
            NET_WriteInt8(packet, diff->cmd.angleturn / 256);
        }
        else
        {
            NET_WriteInt16(packet, diff->cmd.angleturn);
        }
    }
    if (diff->diff & NET_TICDIFF_BUTTONS)
        NET_WriteInt8(packet, diff->cmd.buttons);
    if (diff->diff & NET_TICDIFF_CONSISTANCY)
        NET_WriteInt8(packet, diff->cmd.consistancy);
    if (diff->diff & NET_TICDIFF_CHATCHAR)
        NET_WriteInt8(packet, diff->cmd.chatchar);
    if (diff->diff & NET_TICDIFF_RAVEN)
    {
        NET_WriteInt8(packet, diff->cmd.lookfly);
        NET_WriteInt8(packet, diff->cmd.arti);
    }
    if (diff->diff & NET_TICDIFF_STRIFE)
    {
        NET_WriteInt8(packet, diff->cmd.buttons2);
        NET_WriteInt16(packet, diff->cmd.inventory);
    }
}

boolean NET_ReadTiccmdDiff(net_packet_t *packet, net_ticdiff_t *diff,
                           boolean lowres_turn)
{
    unsigned int val;
    signed int sval;

    // Read header

    if (!NET_ReadInt8(packet, &diff->diff))
        return false;
    
    // Read fields

    if (diff->diff & NET_TICDIFF_FORWARD)
    {
        if (!NET_ReadSInt8(packet, &sval))
            return false;
        diff->cmd.forwardmove = sval;
    }

    if (diff->diff & NET_TICDIFF_SIDE)
    {
        if (!NET_ReadSInt8(packet, &sval))
            return false;
        diff->cmd.sidemove = sval;
    }

    if (diff->diff & NET_TICDIFF_TURN)
    {
        if (lowres_turn)
        {
            if (!NET_ReadSInt8(packet, &sval))
                return false;
            diff->cmd.angleturn = sval * 256;
        }
        else
        {
            if (!NET_ReadSInt16(packet, &sval))
                return false;
            diff->cmd.angleturn = sval;
        }
    }

    if (diff->diff & NET_TICDIFF_BUTTONS)
    {
        if (!NET_ReadInt8(packet, &val))
            return false;
        diff->cmd.buttons = val;
    }

    if (diff->diff & NET_TICDIFF_CONSISTANCY)
    {
        if (!NET_ReadInt8(packet, &val))
            return false;
        diff->cmd.consistancy = val;
    }

    if (diff->diff & NET_TICDIFF_CHATCHAR)
    {
        if (!NET_ReadInt8(packet, &val))
            return false;
        diff->cmd.chatchar = val;
    }

    if (diff->diff & NET_TICDIFF_RAVEN)
    {
        if (!NET_ReadInt8(packet, &val))
            return false;
        diff->cmd.lookfly = val;

        if (!NET_ReadInt8(packet, &val))
            return false;
        diff->cmd.arti = val;
    }

    if (diff->diff & NET_TICDIFF_STRIFE)
    {
        if (!NET_ReadInt8(packet, &val))
            return false;
        diff->cmd.buttons2 = val;

        if (!NET_ReadInt16(packet, &val))
            return false;
        diff->cmd.inventory = val;
    }

    return true;
}

void NET_TiccmdDiff(ticcmd_t *tic1, ticcmd_t *tic2, net_ticdiff_t *diff)
{
    diff->diff = 0;
    diff->cmd = *tic2;

    if (tic1->forwardmove != tic2->forwardmove)
        diff->diff |= NET_TICDIFF_FORWARD;
    if (tic1->sidemove != tic2->sidemove)
        diff->diff |= NET_TICDIFF_SIDE;
    if (tic1->angleturn != tic2->angleturn)
        diff->diff |= NET_TICDIFF_TURN;
    if (tic1->buttons != tic2->buttons)
        diff->diff |= NET_TICDIFF_BUTTONS;
    if (tic1->consistancy != tic2->consistancy)
        diff->diff |= NET_TICDIFF_CONSISTANCY;
    if (tic2->chatchar != 0)
        diff->diff |= NET_TICDIFF_CHATCHAR;

    // Heretic/Hexen-specific

    if (tic1->lookfly != tic2->lookfly || tic2->arti != 0)
        diff->diff |= NET_TICDIFF_RAVEN;

    // Strife-specific

    if (tic1->buttons2 != tic2->buttons2 || tic2->inventory != 0)
        diff->diff |= NET_TICDIFF_STRIFE;
}

void NET_TiccmdPatch(ticcmd_t *src, net_ticdiff_t *diff, ticcmd_t *dest)
{
    memmove(dest, src, sizeof(ticcmd_t));

    // Apply the diff

    if (diff->diff & NET_TICDIFF_FORWARD)
        dest->forwardmove = diff->cmd.forwardmove;
    if (diff->diff & NET_TICDIFF_SIDE)
        dest->sidemove = diff->cmd.sidemove;
    if (diff->diff & NET_TICDIFF_TURN)
        dest->angleturn = diff->cmd.angleturn;
    if (diff->diff & NET_TICDIFF_BUTTONS)
        dest->buttons = diff->cmd.buttons;
    if (diff->diff & NET_TICDIFF_CONSISTANCY)
        dest->consistancy = diff->cmd.consistancy;

    if (diff->diff & NET_TICDIFF_CHATCHAR)
        dest->chatchar = diff->cmd.chatchar;
    else
        dest->chatchar = 0;

    // Heretic/Hexen specific:

    if (diff->diff & NET_TICDIFF_RAVEN)
    {
        dest->lookfly = diff->cmd.lookfly;
        dest->arti = diff->cmd.arti;
    }
    else
    {
        dest->arti = 0;
    }

    // Strife-specific:

    if (diff->diff & NET_TICDIFF_STRIFE)
    {
        dest->buttons2 = diff->cmd.buttons2;
        dest->inventory = diff->cmd.inventory;
    }
    else
    {
        dest->inventory = 0;
    }
}

// 
// net_full_ticcmd_t
// 

boolean NET_ReadFullTiccmd(net_packet_t *packet, net_full_ticcmd_t *cmd, boolean lowres_turn)
{
    unsigned int bitfield;
    int i;

    // Latency

    if (!NET_ReadSInt16(packet, &cmd->latency))
    {
        return false;
    }

    // Regenerate playeringame from the "header" bitfield

    if (!NET_ReadInt8(packet, &bitfield))
    {
        return false;
    }
          
    for (i=0; i<NET_MAXPLAYERS; ++i)
    {
        cmd->playeringame[i] = (bitfield & (1 << i)) != 0;
    }
        
    // Read cmds

    for (i=0; i<NET_MAXPLAYERS; ++i)
    {
        if (cmd->playeringame[i])
        {
            if (!NET_ReadTiccmdDiff(packet, &cmd->cmds[i], lowres_turn))
            {
                return false;
            }
        }
    }

    return true;
}

void NET_WriteFullTiccmd(net_packet_t *packet, net_full_ticcmd_t *cmd, boolean lowres_turn)
{
    unsigned int bitfield;
    int i;

    // Write the latency

    NET_WriteInt16(packet, cmd->latency);

    // Write "header" byte indicating which players are active
    // in this ticcmd

    bitfield = 0;
    
    for (i=0; i<NET_MAXPLAYERS; ++i)
    {
        if (cmd->playeringame[i])
        {
            bitfield |= 1 << i;
        }
    }
    
    NET_WriteInt8(packet, bitfield);

    // Write player ticcmds

    for (i=0; i<NET_MAXPLAYERS; ++i)
    {
        if (cmd->playeringame[i])
        {
            NET_WriteTiccmdDiff(packet, &cmd->cmds[i], lowres_turn);
        }
    }
}

void NET_WriteWaitData(net_packet_t *packet, net_waitdata_t *data)
{
    int i;

    NET_WriteInt8(packet, data->num_players);
    NET_WriteInt8(packet, data->num_drones);
    NET_WriteInt8(packet, data->ready_players);
    NET_WriteInt8(packet, data->max_players);
    NET_WriteInt8(packet, data->is_controller);
    NET_WriteInt8(packet, data->consoleplayer);

    for (i = 0; i < data->num_players && i < NET_MAXPLAYERS; ++i)
    {
        NET_WriteString(packet, data->player_names[i]);
        NET_WriteString(packet, data->player_addrs[i]);
    }

    NET_WriteSHA1Sum(packet, data->wad_sha1sum);
    NET_WriteSHA1Sum(packet, data->deh_sha1sum);
    NET_WriteInt8(packet, data->is_freedoom);
}

boolean NET_ReadWaitData(net_packet_t *packet, net_waitdata_t *data)
{
    int i;
    char *s;

    if (!NET_ReadInt8(packet, (unsigned int *) &data->num_players)
     || !NET_ReadInt8(packet, (unsigned int *) &data->num_drones)
     || !NET_ReadInt8(packet, (unsigned int *) &data->ready_players)
     || !NET_ReadInt8(packet, (unsigned int *) &data->max_players)
     || !NET_ReadInt8(packet, (unsigned int *) &data->is_controller)
     || !NET_ReadSInt8(packet, &data->consoleplayer))
    {
        return false;
    }

    for (i = 0; i < data->num_players && i < NET_MAXPLAYERS; ++i)
    {
        s = NET_ReadString(packet);

        if (s == NULL || strlen(s) >= MAXPLAYERNAME)
        {
            return false;
        }

        M_StringCopy(data->player_names[i], s, MAXPLAYERNAME);

        s = NET_ReadString(packet);

        if (s == NULL || strlen(s) >= MAXPLAYERNAME)
        {
            return false;
        }

        M_StringCopy(data->player_addrs[i], s, MAXPLAYERNAME);
    }

    return NET_ReadSHA1Sum(packet, data->wad_sha1sum)
        && NET_ReadSHA1Sum(packet, data->deh_sha1sum)
        && NET_ReadInt8(packet, (unsigned int *) &data->is_freedoom);
}

static boolean NET_ReadBlob(net_packet_t *packet, uint8_t *buf, size_t len)
{
    unsigned int b;
    int i;

    for (i=0; i<len; ++i)
    {
        if (!NET_ReadInt8(packet, &b))
        {
            return false;
        }

        buf[i] = b;
    }

    return true;
}

static void NET_WriteBlob(net_packet_t *packet, uint8_t *buf, size_t len)
{
    int i;

    for (i=0; i<len; ++i)
    {
        NET_WriteInt8(packet, buf[i]);
    }
}

boolean NET_ReadSHA1Sum(net_packet_t *packet, sha1_digest_t digest)
{
    return NET_ReadBlob(packet, digest, sizeof(sha1_digest_t));
}

void NET_WriteSHA1Sum(net_packet_t *packet, sha1_digest_t digest)
{
    NET_WriteBlob(packet, digest, sizeof(sha1_digest_t));
}

boolean NET_ReadPRNGSeed(net_packet_t *packet, prng_seed_t seed)
{
    return NET_ReadBlob(packet, seed, sizeof(prng_seed_t));
}

void NET_WritePRNGSeed(net_packet_t *packet, prng_seed_t seed)
{
    NET_WriteBlob(packet, seed, sizeof(prng_seed_t));
}

static net_protocol_t ParseProtocolName(const char *name)
{
    int i;

    for (i = 0; i < arrlen(protocol_names); ++i)
    {
        if (!strcmp(protocol_names[i].name, name))
        {
            return protocol_names[i].protocol;
        }
    }

    return NET_PROTOCOL_UNKNOWN;
}

// NET_ReadProtocol reads a single string-format protocol name from the given
// packet, returning NET_PROTOCOL_UNKNOWN if the string describes an unknown
// protocol.
net_protocol_t NET_ReadProtocol(net_packet_t *packet)
{
    const char *name;

    name = NET_ReadString(packet);
    if (name == NULL)
    {
        return NET_PROTOCOL_UNKNOWN;
    }

    return ParseProtocolName(name);
}

// NET_WriteProtocol writes a single string-format protocol name to a packet.
void NET_WriteProtocol(net_packet_t *packet, net_protocol_t protocol)
{
    int i;

    for (i = 0; i < arrlen(protocol_names); ++i)
    {
        if (protocol_names[i].protocol == protocol)
        {
            NET_WriteString(packet, protocol_names[i].name);
            return;
        }
    }

    // If you add an entry to the net_protocol_t enum, a corresponding entry
    // must be added to the protocol_names list.
    I_Error("NET_WriteProtocol: protocol %d missing from protocol_names "
            "list; please add it.", protocol);
}

// NET_ReadProtocolList reads a list of string-format protocol names from
// the given packet, returning a single protocol number. The protocol that is
// returned is the last protocol in the list that is a supported protocol. If
// no recognized protocols are read, NET_PROTOCOL_UNKNOWN is returned.
net_protocol_t NET_ReadProtocolList(net_packet_t *packet)
{
    net_protocol_t result;
    unsigned int num_protocols;
    int i;

    if (!NET_ReadInt8(packet, &num_protocols))
    {
        return NET_PROTOCOL_UNKNOWN;
    }

    result = NET_PROTOCOL_UNKNOWN;

    for (i = 0; i < num_protocols; ++i)
    {
        net_protocol_t p;
        const char *name;

        name = NET_ReadString(packet);
        if (name == NULL)
        {
            return NET_PROTOCOL_UNKNOWN;
        }

        p = ParseProtocolName(name);
        if (p != NET_PROTOCOL_UNKNOWN)
        {
            result = p;
        }
    }

    return result;
}

// NET_WriteProtocolList writes a list of string-format protocol names into
// the given packet, all the supported protocols in the net_protocol_t enum.
// This is slightly different to other functions in this file, in that there
// is nothing the caller can "choose" to write; the built-in list of all
// protocols is always sent.
void NET_WriteProtocolList(net_packet_t *packet)
{
    int i;

    NET_WriteInt8(packet, NET_NUM_PROTOCOLS);

    for (i = 0; i < NET_NUM_PROTOCOLS; ++i)
    {
        NET_WriteProtocol(packet, i);
    }
}

