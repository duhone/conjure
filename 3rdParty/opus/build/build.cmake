set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(INTERFACE_FILES
    ${root}/opus/include/opus.h
    ${root}/opus/include/opus_custom.h
    ${root}/opus/include/opus_defines.h
    ${root}/opus/include/opus_multistream.h
    ${root}/opus/include/opus_projection.h
    ${root}/opus/include/opus_types.h
)

set(SOURCE_FILES
    ${root}/opus/celt/bands.c
    ${root}/opus/celt/celt.c
    ${root}/opus/celt/celt_encoder.c
    ${root}/opus/celt/celt_decoder.c
    ${root}/opus/celt/cwrs.c
    ${root}/opus/celt/entcode.c
    ${root}/opus/celt/entdec.c
    ${root}/opus/celt/entenc.c
    ${root}/opus/celt/kiss_fft.c
    ${root}/opus/celt/laplace.c
    ${root}/opus/celt/mathops.c
    ${root}/opus/celt/mdct.c
    ${root}/opus/celt/modes.c
    ${root}/opus/celt/pitch.c
    ${root}/opus/celt/celt_lpc.c
    ${root}/opus/celt/quant_bands.c
    ${root}/opus/celt/rate.c
    ${root}/opus/celt/vq.c
    ${root}/opus/celt/x86/x86cpu.c
    ${root}/opus/celt/x86/x86_celt_map.c
    ${root}/opus/celt/x86/pitch_sse.c
    ${root}/opus/celt/x86/pitch_sse2.c
    ${root}/opus/celt/x86/pitch_sse4_1.c
    ${root}/opus/celt/x86/vq_sse2.c
    ${root}/opus/celt/x86/celt_lpc_sse4_1.c
    ${root}/opus/celt/x86/pitch_sse4_1.c
#    ${root}/opus/celt/arm/armcpu.c
#    ${root}/opus/celt/arm/arm_celt_map.c
#    ${root}/opus/celt/arm/celt_pitch_xcorr_arm.s
#    ${root}/opus/celt/arm/armopts.s.in
#    ${root}/opus/celt/arm/celt_neon_intr.c
#    ${root}/opus/celt/arm/pitch_neon_intr.c
#    ${root}/opus/celt/arm/celt_ne10_fft.c
#    ${root}/opus/celt/arm/celt_ne10_mdct.c
    ${root}/opus/silk/CNG.c
    ${root}/opus/silk/code_signs.c
    ${root}/opus/silk/init_decoder.c
    ${root}/opus/silk/decode_core.c
    ${root}/opus/silk/decode_frame.c
    ${root}/opus/silk/decode_parameters.c
    ${root}/opus/silk/decode_indices.c
    ${root}/opus/silk/decode_pulses.c
    ${root}/opus/silk/decoder_set_fs.c
    ${root}/opus/silk/dec_API.c
    ${root}/opus/silk/enc_API.c
    ${root}/opus/silk/encode_indices.c
    ${root}/opus/silk/encode_pulses.c
    ${root}/opus/silk/gain_quant.c
    ${root}/opus/silk/interpolate.c
    ${root}/opus/silk/LP_variable_cutoff.c
    ${root}/opus/silk/NLSF_decode.c
    ${root}/opus/silk/NSQ.c
    ${root}/opus/silk/NSQ_del_dec.c
    ${root}/opus/silk/PLC.c
    ${root}/opus/silk/shell_coder.c
    ${root}/opus/silk/tables_gain.c
    ${root}/opus/silk/tables_LTP.c
    ${root}/opus/silk/tables_NLSF_CB_NB_MB.c
    ${root}/opus/silk/tables_NLSF_CB_WB.c
    ${root}/opus/silk/tables_other.c
    ${root}/opus/silk/tables_pitch_lag.c
    ${root}/opus/silk/tables_pulses_per_block.c
    ${root}/opus/silk/VAD.c
    ${root}/opus/silk/control_audio_bandwidth.c
    ${root}/opus/silk/quant_LTP_gains.c
    ${root}/opus/silk/VQ_WMat_EC.c
    ${root}/opus/silk/HP_variable_cutoff.c
    ${root}/opus/silk/NLSF_encode.c
    ${root}/opus/silk/NLSF_VQ.c
    ${root}/opus/silk/NLSF_unpack.c
    ${root}/opus/silk/NLSF_del_dec_quant.c
    ${root}/opus/silk/process_NLSFs.c
    ${root}/opus/silk/stereo_LR_to_MS.c
    ${root}/opus/silk/stereo_MS_to_LR.c
    ${root}/opus/silk/check_control_input.c
    ${root}/opus/silk/control_SNR.c
    ${root}/opus/silk/init_encoder.c
    ${root}/opus/silk/control_codec.c
    ${root}/opus/silk/A2NLSF.c
    ${root}/opus/silk/ana_filt_bank_1.c
    ${root}/opus/silk/biquad_alt.c
    ${root}/opus/silk/bwexpander_32.c
    ${root}/opus/silk/bwexpander.c
    ${root}/opus/silk/debug.c
    ${root}/opus/silk/decode_pitch.c
    ${root}/opus/silk/inner_prod_aligned.c
    ${root}/opus/silk/lin2log.c
    ${root}/opus/silk/log2lin.c
    ${root}/opus/silk/LPC_analysis_filter.c
    ${root}/opus/silk/LPC_inv_pred_gain.c
    ${root}/opus/silk/table_LSF_cos.c
    ${root}/opus/silk/NLSF2A.c
    ${root}/opus/silk/NLSF_stabilize.c
    ${root}/opus/silk/NLSF_VQ_weights_laroia.c
    ${root}/opus/silk/pitch_est_tables.c
    ${root}/opus/silk/resampler.c
    ${root}/opus/silk/resampler_down2_3.c
    ${root}/opus/silk/resampler_down2.c
    ${root}/opus/silk/resampler_private_AR2.c
    ${root}/opus/silk/resampler_private_down_FIR.c
    ${root}/opus/silk/resampler_private_IIR_FIR.c
    ${root}/opus/silk/resampler_private_up2_HQ.c
    ${root}/opus/silk/resampler_rom.c
    ${root}/opus/silk/sigm_Q15.c
    ${root}/opus/silk/sort.c
    ${root}/opus/silk/sum_sqr_shift.c
    ${root}/opus/silk/stereo_decode_pred.c
    ${root}/opus/silk/stereo_encode_pred.c
    ${root}/opus/silk/stereo_find_predictor.c
    ${root}/opus/silk/stereo_quant_pred.c
    ${root}/opus/silk/LPC_fit.c
    ${root}/opus/silk/x86/NSQ_sse4_1.c
    ${root}/opus/silk/x86/NSQ_del_dec_sse4_1.c
    ${root}/opus/silk/x86/x86_silk_map.c
    ${root}/opus/silk/x86/VAD_sse4_1.c
    ${root}/opus/silk/x86/VQ_WMat_EC_sse4_1.c
#    ${root}/opus/silk/arm/arm_silk_map.c
#    ${root}/opus/silk/arm/biquad_alt_neon_intr.c
#    ${root}/opus/silk/arm/LPC_inv_pred_gain_neon_intr.c
#    ${root}/opus/silk/arm/NSQ_del_dec_neon_intr.c
#    ${root}/opus/silk/arm/NSQ_neon.c
#    ${root}/opus/silk/fixed/LTP_analysis_filter_FIX.c
#    ${root}/opus/silk/fixed/LTP_scale_ctrl_FIX.c
#    ${root}/opus/silk/fixed/corrMatrix_FIX.c
#    ${root}/opus/silk/fixed/encode_frame_FIX.c
#    ${root}/opus/silk/fixed/find_LPC_FIX.c
#    ${root}/opus/silk/fixed/find_LTP_FIX.c
#    ${root}/opus/silk/fixed/find_pitch_lags_FIX.c
#    ${root}/opus/silk/fixed/find_pred_coefs_FIX.c
#    ${root}/opus/silk/fixed/noise_shape_analysis_FIX.c
#    ${root}/opus/silk/fixed/process_gains_FIX.c
#    ${root}/opus/silk/fixed/regularize_correlations_FIX.c
#    ${root}/opus/silk/fixed/residual_energy16_FIX.c
#    ${root}/opus/silk/fixed/residual_energy_FIX.c
#    ${root}/opus/silk/fixed/warped_autocorrelation_FIX.c
#    ${root}/opus/silk/fixed/apply_sine_window_FIX.c
#    ${root}/opus/silk/fixed/autocorr_FIX.c
#    ${root}/opus/silk/fixed/burg_modified_FIX.c
#    ${root}/opus/silk/fixed/k2a_FIX.c
#    ${root}/opus/silk/fixed/k2a_Q16_FIX.c
#    ${root}/opus/silk/fixed/pitch_analysis_core_FIX.c
#    ${root}/opus/silk/fixed/vector_ops_FIX.c
#    ${root}/opus/silk/fixed/schur64_FIX.c
#    ${root}/opus/silk/fixed/schur_FIX.c
#    ${root}/opus/silk/fixed/x86/vector_ops_FIX_sse.c
#    ${root}/opus/silk/fixed/x86/burg_modified_FIX_sse.c
#    ${root}/opus/silk/fixed/arm/warped_autocorrelation_FIX_neon_intr.c
    ${root}/opus/silk/float/apply_sine_window_FLP.c
    ${root}/opus/silk/float/corrMatrix_FLP.c
    ${root}/opus/silk/float/encode_frame_FLP.c
    ${root}/opus/silk/float/find_LPC_FLP.c 
    ${root}/opus/silk/float/find_LTP_FLP.c
    ${root}/opus/silk/float/find_pitch_lags_FLP.c
    ${root}/opus/silk/float/find_pred_coefs_FLP.c
    ${root}/opus/silk/float/LPC_analysis_filter_FLP.c
    ${root}/opus/silk/float/LTP_analysis_filter_FLP.c
    ${root}/opus/silk/float/LTP_scale_ctrl_FLP.c
    ${root}/opus/silk/float/noise_shape_analysis_FLP.c
    ${root}/opus/silk/float/process_gains_FLP.c
    ${root}/opus/silk/float/regularize_correlations_FLP.c
    ${root}/opus/silk/float/residual_energy_FLP.c
    ${root}/opus/silk/float/warped_autocorrelation_FLP.c
    ${root}/opus/silk/float/wrappers_FLP.c
    ${root}/opus/silk/float/autocorrelation_FLP.c
    ${root}/opus/silk/float/burg_modified_FLP.c
    ${root}/opus/silk/float/bwexpander_FLP.c
    ${root}/opus/silk/float/energy_FLP.c
    ${root}/opus/silk/float/inner_product_FLP.c
    ${root}/opus/silk/float/k2a_FLP.c
    ${root}/opus/silk/float/LPC_inv_pred_gain_FLP.c
    ${root}/opus/silk/float/pitch_analysis_core_FLP.c
    ${root}/opus/silk/float/scale_copy_vector_FLP.c
    ${root}/opus/silk/float/scale_vector_FLP.c
    ${root}/opus/silk/float/schur_FLP.c
    ${root}/opus/silk/float/sort_FLP.c
	
    ${root}/opus/config.h
    ${root}/opus/src/opus_private.h
    ${root}/opus/src/analysis.h
    ${root}/opus/src/mlp.h
    ${root}/opus/src/tansig_table.h
	
    ${root}/opus/src/opus.c
    ${root}/opus/src/opus_decoder.c
    ${root}/opus/src/opus_encoder.c
    ${root}/opus/src/opus_multistream.c
    ${root}/opus/src/opus_multistream_encoder.c
    ${root}/opus/src/opus_multistream_decoder.c
    ${root}/opus/src/repacketizer.c
    ${root}/opus/src/analysis.c
    ${root}/opus/src/mlp.c
    ${root}/opus/src/mlp_data.c
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(opus OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(opus)

target_compile_definitions(opus PRIVATE HAVE_CONFIG_H=1)

target_include_directories(opus PRIVATE "${root}/opus/include")
target_include_directories(opus PRIVATE "${root}/opus/celt")
target_include_directories(opus PRIVATE "${root}/opus/silk")
target_include_directories(opus PRIVATE "${root}/opus/silk/float")
target_include_directories(opus SYSTEM PUBLIC "${root}/opus")
