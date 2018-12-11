// Ported to a NIF by Alex Dovzhanyn for the Elixium Network

#include <erl_nif.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "itsuku.h"
#include <iostream>
#include <string.h>
#include <cstring>


template<uint N>
// Converts an array of uint32 elements to an erlang list of integers
void convert_uint_array_to_list(ErlNifEnv* env, uint (&arr)[N], ERL_NIF_TERM &var) {
  std::vector<ERL_NIF_TERM> term_array;
  int count = 0;
  for(const uint32_t &item : arr) {
    term_array.push_back(enif_make_ulong(env, item));
    count++;
  }

  var = enif_make_list_from_array(env, term_array.data(), count);
}

template<uint N>
// Converts an array of uint8 elements to an erlang list of integers
void convert_uint8_array_to_list(ErlNifEnv* env, uint8_t (&arr)[N], ERL_NIF_TERM &var) {
  std::vector<ERL_NIF_TERM> term_array;
  int count = 0;

  for(const uint32_t &item : arr) {
    term_array.push_back(enif_make_ulong(env, item));
    count++;
  }

  var = enif_make_list_from_array(env, term_array.data(), count);
}

void convert_uint8_vector_to_list(ErlNifEnv* env, std::vector<uint8_t> arr, ERL_NIF_TERM &var) {
  std::vector<ERL_NIF_TERM> term_array;
  int count = 0;

  for(const uint32_t &item : arr) {
    term_array.push_back(enif_make_ulong(env, item));
    count++;
  }

  var = enif_make_list_from_array(env, term_array.data(), count);
}

void convert_opening_vector_to_list(ErlNifEnv* env, std::vector<t_istuku_opening_el> vector, ERL_NIF_TERM &var) {
  std::vector<ERL_NIF_TERM> term_array;
  int count = 0;

  for (const t_istuku_opening_el &item : vector) {
    ERL_NIF_TERM idx = enif_make_ulong(env, item.idx);
    ERL_NIF_TERM hash;
    ERL_NIF_TERM map = enif_make_new_map(env);

    convert_uint8_vector_to_list(env, item.hash, hash);

    enif_make_map_put(env, map, enif_make_atom(env, "idx"), idx, &map);
    enif_make_map_put(env, map, enif_make_atom(env, "hash"), hash, &map);

    term_array.push_back(map);
    count++;
  }

  var = enif_make_list_from_array(env, term_array.data(), count);
}

template<uint N>
void convert_list_to_uint_array(ErlNifEnv* env, ERL_NIF_TERM tail, uint8_t (&var)[N]) {
  uint length;
  ERL_NIF_TERM head;

  enif_get_list_length(env, tail, &length);

  for (int i = 0; i < length; i++) {
    uint val;
    enif_get_list_cell(env, tail, &head, &tail);
    enif_get_uint(env, head, &val);
    var[i] = val;
  }
}

template<uint N>
void convert_list_to_uint_array(ErlNifEnv* env, ERL_NIF_TERM tail, uint32_t (&var)[N]) {
  uint length;
  ERL_NIF_TERM head;

  enif_get_list_length(env, tail, &length);

  for (int i = 0; i < length; i++) {
    uint val;
    enif_get_list_cell(env, tail, &head, &tail);
    enif_get_uint(env, head, &val);
    var[i] = val;
  }
}

void convert_list_to_uint_vector(ErlNifEnv* env, ERL_NIF_TERM tail, std::vector<uint8_t> (&var)) {
  uint length;
  ERL_NIF_TERM head;

  enif_get_list_length(env, tail, &length);

  for (int i = 0; i < length; i++) {
    uint val;
    enif_get_list_cell(env, tail, &head, &tail);
    enif_get_uint(env, head, &val);
    var.push_back(val);
  }
}

void convert_opening_list_to_vector(ErlNifEnv* env, ERL_NIF_TERM tail, std::vector<t_istuku_opening_el> &var) {
  uint length;
  ERL_NIF_TERM head;

  enif_get_list_length(env, tail, &length);

  for (int i = 0; i < length; i++) {
    enif_get_list_cell(env, tail, &head, &tail);

    ERL_NIF_TERM idx;
    ERL_NIF_TERM hash;
    t_istuku_opening_el opening;

    enif_get_map_value(env, head, enif_make_atom(env, "idx"), &idx);
    enif_get_map_value(env, head, enif_make_atom(env, "hash"), &hash);

    enif_get_uint(env, idx, &opening.idx);
    convert_list_to_uint_vector(env, hash, opening.hash);

    var.push_back(opening);
  }
}

ERL_NIF_TERM itsuku_pow_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  int difficulty = 0;
  ErlNifBinary input;
  t_istuku_config config;
  t_istuku_proof proof;

  if (!enif_inspect_binary(env, argv[0], &input)) {
    return enif_make_badarg(env);
  }
  if (!enif_get_int(env, argv[1], &difficulty)) {
    return enif_make_badarg(env);
  }

  uint8_t *I = (uint8_t*) malloc(config.suku_Il); //challenge
  memset(I, 0, config.suku_Il);
  memcpy(I, input.data, input.size );

  itsuku_pow(config, I, difficulty, &proof);

  free(I);

  ERL_NIF_TERM result_proof = enif_make_new_map(env);
  ERL_NIF_TERM proof_n = enif_make_ulong(env, proof.N);
  ERL_NIF_TERM proof_Lp;
  ERL_NIF_TERM proof_Fp;
  ERL_NIF_TERM proof_Iij;
  ERL_NIF_TERM proof_Lp_idxs;

  convert_uint8_array_to_list(env, proof.Lp, proof_Lp);
  convert_uint_array_to_list(env, proof.Iij, proof_Iij);
  convert_uint_array_to_list(env, proof.Lp_idxs, proof_Lp_idxs);
  convert_opening_vector_to_list(env, proof.Fp, proof_Fp);

  enif_make_map_put(env, result_proof, enif_make_atom(env, "n"), proof_n, &result_proof);
  enif_make_map_put(env, result_proof, enif_make_atom(env, "Lp"), proof_Lp, &result_proof);
  enif_make_map_put(env, result_proof, enif_make_atom(env, "Fp"), proof_Fp, &result_proof);
  enif_make_map_put(env, result_proof, enif_make_atom(env, "Iij"), proof_Iij, &result_proof);
  enif_make_map_put(env, result_proof, enif_make_atom(env, "Lp_idxs"), proof_Lp_idxs, &result_proof);

  return result_proof;
}

ERL_NIF_TERM itsuku_verify_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  int difficulty = 0;
  ErlNifBinary identifier;
  t_istuku_config config;
  t_istuku_proof proof;

  if (!enif_inspect_binary(env, argv[0], &identifier)) {
    return enif_make_badarg(env);
  }
  if (!enif_get_int(env, argv[1], &difficulty)) {
    return enif_make_badarg(env);
  }
  if (!enif_is_map(env, argv[2])) {
    return enif_make_badarg(env);
  }

  uint8_t *I = (uint8_t*) malloc(config.suku_Il); //challenge
  memset(I, 0, config.suku_Il);
  memcpy(I, identifier.data, identifier.size );

  ERL_NIF_TERM proof_n;
  ERL_NIF_TERM proof_Lp;
  ERL_NIF_TERM proof_Fp;
  ERL_NIF_TERM proof_Iij;
  ERL_NIF_TERM proof_Lp_idxs;

  enif_get_map_value(env, argv[2], enif_make_atom(env, "n"), &proof_n);
  enif_get_map_value(env, argv[2], enif_make_atom(env, "Lp"), &proof_Lp);
  enif_get_map_value(env, argv[2], enif_make_atom(env, "Fp"), &proof_Fp);
  enif_get_map_value(env, argv[2], enif_make_atom(env, "Iij"), &proof_Iij);
  enif_get_map_value(env, argv[2], enif_make_atom(env, "Lp_idxs"), &proof_Lp_idxs);

  enif_get_uint(env, proof_n, &proof.N);
  convert_list_to_uint_array(env, proof_Lp, proof.Lp);
  convert_list_to_uint_array(env, proof_Iij, proof.Iij);
  convert_list_to_uint_array(env, proof_Lp_idxs, proof.Lp_idxs);
  convert_opening_list_to_vector(env, proof_Fp, proof.Fp);

  bool valid = itsuku_verify(config, I , difficulty, &proof);

  free(I);

  return enif_make_atom(env, valid ? "true" : "false");
}

ErlNifFunc nif_funcs[] =
{
    {"pow", 2, itsuku_pow_nif},
    {"verify", 3, itsuku_verify_nif}
};

ERL_NIF_INIT(Elixir.Exmdbx, nif_funcs, nullptr, nullptr, nullptr, nullptr);
