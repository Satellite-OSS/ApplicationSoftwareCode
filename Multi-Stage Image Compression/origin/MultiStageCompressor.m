% Multi-stage image compression MATLAB implementation
% Class definition file

classdef MultiStageCompressor < handle
    properties
        original            % Original image
        original_size       % Original image size (bytes)
        stages              % Compression stage configuration
    end

    methods
        function obj = MultiStageCompressor(input_path)
            % Constructor, initializes the compressor

            % ------------- BEGIN DEBUGGING CODE -------------
            fprintf('DEBUG (MultiStageCompressor): Constructor called with input_path = "%s"\n', input_path);
            fprintf('DEBUG (MultiStageCompressor): Current MATLAB pwd before any operation: %s\n', pwd);

            % Also check with exist() again inside the constructor
            if ~exist(input_path, 'file')
                fprintf('ERROR (MultiStageCompressor): exist(''%s'', ''file'') FAILED inside constructor.\n', input_path);
                % If exist() fails here, dir() will almost certainly fail too
                error('MultiStageCompressor:InputFileNotFoundByExist', ...
                      'Input file "%s" not found by exist() check inside constructor. Current pwd: %s', input_path, pwd);
            else
                fprintf('DEBUG (MultiStageCompressor): exist(''%s'', ''file'') PASSED inside constructor.\n', input_path);
            end
            % ------------- END DEBUGGING CODE ---------------

            obj.original = obj.load_image(input_path); % imread might error out first if the file is truly inaccessible

            % ------------- BEGIN DEBUGGING CODE for dir() -------------
            fprintf('DEBUG (MultiStageCompressor): Calling dir(''%s'')...\n', input_path);
            file_info = dir(input_path);

            if isempty(file_info)
                fprintf('ERROR (MultiStageCompressor): dir(''%s'') returned an EMPTY struct.\n', input_path);
                fprintf('DEBUG (MultiStageCompressor): Listing contents of MATLAB pwd (%s) from MATLAB:\n', pwd);
                try
                    ls(pwd); % or dir(pwd)
                catch ME_ls_pwd
                    fprintf('DEBUG (MultiStageCompressor): Could not list pwd contents: %s\n', ME_ls_pwd.message);
                end
                fprintf('DEBUG (MultiStageCompressor): Listing contents of /app (expected image location) from MATLAB:\n');
                try
                    ls /app; % or dir('/app')
                catch ME_ls_app
                    fprintf('DEBUG (MultiStageCompressor): Could not list /app contents: %s\n', ME_ls_app.message);
                end
                error('MultiStageCompressor:FileNotFoundByDir', ...
                      'Failed to get file information using dir() for "%s". dir() returned empty. Check paths and permissions. Current pwd: %s', input_path, pwd);
            elseif ~isfield(file_info(1), 'bytes') % Check if the first returned struct has a 'bytes' field
                 fprintf('ERROR (MultiStageCompressor): dir(''%s'') did not return a struct with a "bytes" field for its first element.\n', input_path);
                 disp('DEBUG (MultiStageCompressor): Structure of file_info(1):');
                 disp(file_info(1)); % Display the content of the struct returned by dir
                 error('MultiStageCompressor:MissingBytesField', ...
                      'dir() for "%s" did not return a "bytes" field in the expected structure.', input_path);
            end

            % If the check passes, then file_info(1).bytes should be safe
            obj.original_size = file_info(1).bytes; % Use (1) to ensure the first is taken even if multiple are returned
            fprintf('DEBUG (MultiStageCompressor): Successfully got file_info(1).bytes = %d\n', obj.original_size);
            % ------------- END DEBUGGING CODE for dir() -------------

            % Define compression stages (your original code)
            obj.stages = struct();
            obj.stages(1).target_ratio = 0.8;
            obj.stages(1).method = "jpeg";
            obj.stages(2).target_ratio = 0.7;
            obj.stages(2).method = "webp";
            obj.stages(3).target_ratio = 0.5;
            obj.stages(3).method = "quantized_jpeg";
            obj.stages(4).target_ratio = 0.3;
            obj.stages(4).method = "residual_compression";
            obj.stages(5).target_ratio = 0.1;
            obj.stages(5).method = "deep_compression";
        end

        function img = load_image(~, path)
            % Load image
            fprintf('DEBUG (MultiStageCompressor-load_image): Attempting to imread(''%s'')...\n', path);
            if ~exist(path, 'file') % Also add an exist check before imread
                 fprintf('ERROR (MultiStageCompressor-load_image): exist() FAILED for "%s" just before imread.\n', path);
                 % Could also consider throwing an error directly here, as imread is likely to fail
                 % error('MultiStageCompressor:LoadImageFileNotFound', 'File "%s" not found by exist() before imread.', path);
            end
            img = imread(path);
            fprintf('DEBUG (MultiStageCompressor-load_image): imread(''%s'') completed.\n', path);
        end

        function [data, quality_info] = adaptive_compress(obj, stage_num)
            % Adaptive compression, select appropriate quality parameters based on target compression ratio
            config = obj.stages(stage_num);

            % Select different compression functions based on the compression method
            if config.method == "jpeg"
                [data, quality_info] = obj.binary_search_compression(@(t, q) obj.jpeg_compression(t, q), 1, 100, 0.05, stage_num);
            elseif config.method == "webp"
                [data, quality_info] = obj.binary_search_compression(@(t, q) obj.webp_compression(t), 1, 100, 0.05, stage_num);
            elseif config.method == "quantized_jpeg"
                [data, quality_info] = obj.binary_search_compression(@(t, q) obj.quantized_jpeg(t), 1, 100, 0.05, stage_num);
            elseif config.method == "residual_compression"
                [data, quality_info] = obj.binary_search_compression(@(t, q) obj.residual_compression(stage_num, t), 1, 100, 0.05, stage_num);
            elseif config.method == "deep_compression"
                [data, quality_info] = obj.binary_search_compression(@(t, q) obj.deep_compression(t), 1, 100, 0.05, stage_num);
            end
        end

        function [best_data, best_quality] = binary_search_compression(obj, compress_func, low, high, tolerance, stage_num)
            % Binary search for optimal compression quality
            best_quality = high;
            best_data = [];
            best_size = inf;
            config_target_ratio = obj.stages(stage_num).target_ratio; % Use the target ratio of the current stage

            % Calculate target size
            target_size = max(config_target_ratio * obj.original_size, 7 * 7 * 3);

            while low <= high
                quality = floor((low + high) / 2);
                if quality < 1 % Ensure quality does not go below valid range (e.g., 1 for JPEG)
                    quality = 1;
                end

                [data, ~] = compress_func(target_size, quality);

                if isempty(data) % If the compression function returns empty data, it might mean compression failed
                    fprintf('WARN (binary_search_compression): compress_func returned empty data for quality %d\n', quality);
                    % Handle this situation, e.g., try different quality or skip
                    if quality == low && low == high % Avoid infinite loop
                        break;
                    end
                    if target_size > 0 % Assume larger files need lower quality (stronger compression)
                        high = quality -1;
                    else % Assume smaller files can have higher quality
                        low = quality + 1;
                    end
                    if low > high % Ensure the loop will terminate
                        break;
                    end
                    continue;
                end

                current_size = length(data);
                current_ratio = current_size / obj.original_size;

                % Check if within the tolerance range of the target ratio
                if abs(current_ratio - config_target_ratio) <= tolerance
                    best_data = data; % Update best_data because the condition is met
                    best_quality = quality;
                    return;
                end

                % Store the best result
                if abs(current_ratio - config_target_ratio) < abs(best_size / obj.original_size - config_target_ratio)
                    best_quality = quality;
                    best_data = data;
                    best_size = current_size;
                end

                if current_size > target_size
                    high = quality - 1;
                else
                    low = quality + 1;
                end
            end
            % If the loop ends without returning, ensure best_data is not the initial empty value (if there was at least one valid compression)
            if isempty(best_data) && exist('data','var') && ~isempty(data) % If the last attempt had data
                 best_data = data; % Use data from the last attempt
                 % best_quality is already the quality of the last attempt
            end
        end

        function [data, quality] = jpeg_compression(obj, target_size, initial_quality)
            % JPEG compression implementation
            if nargin < 3
                initial_quality = 95;
            end

            quality = initial_quality;
            data = []; % Initialize data

            while quality > 5
                % Create temporary file for compression
                temp_file = [tempname '.jpg'];
                try
                    imwrite(obj.original, temp_file, 'jpg', 'Quality', quality);

                    % Get file size
                    file_info = dir(temp_file);
                    if isempty(file_info) || ~isfield(file_info(1), 'bytes')
                        fprintf('WARN (jpeg_compression): Could not get file info for temp file %s\n', temp_file);
                        delete(temp_file); % Clean up
                        quality = quality - 5; % Try lowering quality
                        continue;
                    end
                    file_size = file_info(1).bytes;

                    % Read compressed data
                    fid = fopen(temp_file, 'rb');
                    if fid == -1
                        fprintf('WARN (jpeg_compression): Could not open temp file %s for reading\n', temp_file);
                        delete(temp_file);
                        quality = quality - 5;
                        continue;
                    end
                    data_read = fread(fid, inf, '*uint8');
                    fclose(fid);
                    delete(temp_file); % Ensure deletion

                    data = data_read; % Update data

                    if file_size <= target_size * 1.05 % Relax tolerance slightly
                        break;
                    end
                catch ME_imwrite
                    fprintf('ERROR (jpeg_compression): imwrite failed for quality %d: %s\n', quality, ME_imwrite.message);
                    if exist(temp_file, 'file')
                        delete(temp_file);
                    end
                    % If imwrite fails, the original image might be problematic or quality parameter invalid
                    % Can choose to break or try lower quality
                end

                quality = quality - 5;
            end
            if isempty(data) % If data is still empty after the loop, compression failed or did not meet target
                fprintf('WARN (jpeg_compression): Could not compress to target size, using last valid or fallback.\n');
                % Can choose to return a default compressed result or empty, depending on how the upper layer handles it
            end
        end

        function [data, quality] = webp_compression(obj, target_size)
            % WebP compression implementation
            % Note: MATLAB may require additional toolboxes or external calls to support WebP
            % Here JPEG is used as a substitute; in practical applications, system calls or MEX functions can be used
            fprintf('INFO (webp_compression): WebP not natively supported, using JPEG as fallback.\n');
            [data, quality] = obj.jpeg_compression(target_size, 90); % Use JPEG logic
        end

        function [data, quality_info] = quantized_jpeg(obj, target_size)
            % Quantization table optimized JPEG compression
            % In MATLAB, advanced options of imwrite can be used to customize quantization tables
            % Standard JPEG compression is used as a substitute here
            fprintf('INFO (quantized_jpeg): Custom quantization not implemented, using standard JPEG as fallback.\n');
            [data, quality] = obj.jpeg_compression(target_size, 90);
            if ~isempty(data)
                quality_info = ['quality=' num2str(quality) ', fallback_jpeg'];
            else
                quality_info = 'min_quality_reached_fallback';
            end
        end

        function [data, quality_info] = residual_compression(obj, stage_num, target_size)
            % Residual compression implementation - strictly following Python version
            quality_info = 'residual_failed_fallback_jpeg'; % Default value
            try
                prev_stage = stage_num - 1;
                if prev_stage < 1 || ~isfield(obj.stages(prev_stage), 'data') || isempty(obj.stages(prev_stage).data)
                    error('Previous stage data does not exist or is empty');
                end

                prev_data = obj.stages(prev_stage).data;

                temp_prev_file = [tempname '.tmp']; % Use .tmp or other non-image extension to avoid imread confusion
                fid = fopen(temp_prev_file, 'wb');
                fwrite(fid, prev_data);
                fclose(fid);

                prev_img = imread(temp_prev_file); % Try to decode with imread
                delete(temp_prev_file);

                if isempty(prev_img)
                    error('Previous stage image decoding failed (imread returned empty)');
                end

                [h_orig, w_orig, ~] = size(obj.original);
                if size(prev_img, 1) ~= h_orig || size(prev_img, 2) ~= w_orig
                    prev_img = imresize(prev_img, [h_orig, w_orig]);
                end

                residual = int16(obj.original) - int16(prev_img);

                % MATLAB does not have a direct zlib.compress. This is a complex alternative, may not be fully equivalent.
                % Java's DeflaterOutputStream can be used if available & allowed.
                % Simple alternative: no compression or use a very lightweight method.
                % For demonstration, we assume residual is directly used as data (this won't compress)
                % Or, if the goal is to simulate Python's zlib, more complex Java calls or MEX are needed
                % residual_compressed_bytes = typecast(residual(:), 'uint8'); % Minimal example: no compression

                % Try using Java Deflater (if MATLAB environment allows and includes relevant libraries)
                try
                    j_residual_bytes = typecast(residual(:), 'uint8');
                    byte_array_output_stream = java.io.ByteArrayOutputStream();
                    deflater_output_stream = java.util.zip.DeflaterOutputStream(byte_array_output_stream);
                    deflater_output_stream.write(j_residual_bytes);
                    deflater_output_stream.close();
                    residual_compressed = byte_array_output_stream.toByteArray();
                    residual_compressed = typecast(residual_compressed, 'uint8'); % Ensure it's uint8 column vector
                catch ME_java_deflate
                    fprintf('WARN (residual_compression): Java Deflater failed: %s. Using uncompressed residual.\n', ME_java_deflate.message);
                    residual_compressed = typecast(residual(:), 'uint8'); % Fallback: uncompressed
                end


                total_size = length(prev_data) + length(residual_compressed);
                if total_size <= target_size
                    metadata = typecast(uint32(length(prev_data)), 'uint8'); % Ensure column vector
                    if ~iscolumn(metadata) metadata = metadata'; end
                    if ~iscolumn(prev_data) prev_data = prev_data'; end
                    if ~iscolumn(residual_compressed) residual_compressed = residual_compressed'; end

                    data = [metadata; prev_data; residual_compressed];
                    quality_info = 'residual_success';
                    return;
                end

                fprintf('INFO (residual_compression): Residual + prev_data too large. Falling back to JPEG.\n');
                [data, quality_info] = obj.jpeg_compression(target_size);

            catch e
                fprintf('ERROR (residual_compression): %s. Falling back to JPEG.\n', e.message);
                [data, quality_info] = obj.jpeg_compression(target_size);
            end
        end

        function [data, quality_info] = deep_compression(obj, target_size)
            % Optimized deep compression implementation
            quality_info = 'deep_failed_fallback_jpeg'; % Default
            try
                min_scale = 0.1;
                max_scale = 1.0;
                best_data = [];
                best_size_achieved = inf; % Store the size of best_data
                best_quality_achieved = 0;
                best_scale_achieved = 0;

                for i = 1:5 % Iterative search for scale
                    current_scale = (min_scale + max_scale) / 2;

                    [h_orig, w_orig, ~] = size(obj.original);
                    new_w = max(7, round(w_orig * current_scale));
                    new_h = max(7, round(h_orig * current_scale));
                    small_img = imresize(obj.original, [new_h, new_w]);

                    found_better_in_iteration = false;
                    for quality = [95, 85, 75, 65, 55, 45, 35, 25, 15, 5] % Try more qualities
                        temp_file = [tempname '.jpg'];
                        try
                            imwrite(small_img, temp_file, 'jpg', 'Quality', quality);
                            file_info_temp = dir(temp_file);

                            if isempty(file_info_temp) || ~isfield(file_info_temp(1), 'bytes')
                                delete(temp_file);
                                continue;
                            end
                            current_file_size = file_info_temp(1).bytes;

                            % Python logic: if size <= target_size and (best_data is None or size > best_size):
                            if current_file_size <= target_size
                                if isempty(best_data) || current_file_size > best_size_achieved % Try to get largest file under target
                                    fid = fopen(temp_file, 'rb');
                                    best_data = fread(fid, inf, '*uint8');
                                    fclose(fid);
                                    best_size_achieved = current_file_size;
                                    best_quality_achieved = quality;
                                    best_scale_achieved = current_scale;
                                    found_better_in_iteration = true;
                                end
                            end
                            delete(temp_file);
                        catch ME_deep_imwrite
                             fprintf('WARN (deep_compression): imwrite failed for scale %f, quality %d: %s\n', current_scale, quality, ME_deep_imwrite.message);
                            if exist(temp_file, 'file'), delete(temp_file); end
                        end
                    end % End quality loop

                    if found_better_in_iteration && best_size_achieved <= target_size
                        min_scale = current_scale; % Try to improve by increasing scale (and thus size)
                    else
                        max_scale = current_scale; % Current scale is too large or no improvement, try smaller
                    end

                    % Python logic: if best_data is not None and best_size >= 0.9 * target_size: break
                    if ~isempty(best_data) && best_size_achieved >= 0.9 * target_size && best_size_achieved <= target_size
                        break; % Good enough result found
                    end
                end % End scale search loop

                if isempty(best_data)
                    fprintf('INFO (deep_compression): No suitable parameters found. Falling back to basic JPEG.\n');
                    [data, quality_info] = obj.jpeg_compression(target_size, 35); % Fallback quality
                    return;
                end

                data = best_data;
                quality_info = sprintf('deep_scale=%.2f_qual=%d_size=%d', best_scale_achieved, best_quality_achieved, best_size_achieved);

            catch e
                fprintf('ERROR (deep_compression): %s. Falling back to JPEG.\n', e.message);
                [data, quality_info] = obj.jpeg_compression(target_size);
            end
        end

        function results = execute(obj)
            % Execute multi-stage compression
            results = repmat(struct(... % Initialize struct array
                'target_ratio', 0, ...
                'actual_ratio', 0, ...
                'compressed_size', 0, ...
                'params', '', ...
                'data', [], ...
                'ssim', 0, ...
                'psnr', 0 ...
            ), 1, 5);

            if isempty(obj.original) || obj.original_size == 0
                error('Original image not loaded or zero size, cannot execute compression.');
            end

            for stage = 1:length(obj.stages) % Iterate through defined stages
                fprintf('\n--- Executing Stage %d: Method %s ---\n', stage, obj.stages(stage).method);
                [compressed_data, params] = obj.adaptive_compress(stage);

                if isempty(compressed_data)
                    fprintf('WARN (execute): Stage %d compression returned empty data. Skipping metrics and saving.\n', stage);
                    % Populate results with what we have
                    results(stage).target_ratio = obj.stages(stage).target_ratio;
                    results(stage).actual_ratio = NaN;
                    results(stage).compressed_size = 0;
                    results(stage).params = params;
                    results(stage).data = [];
                    results(stage).ssim = NaN;
                    results(stage).psnr = NaN;
                    obj.stages(stage).data = []; % Store empty data
                    continue; % Move to next stage
                end

                obj.stages(stage).data = compressed_data; % Store for potential use in residual
                curr_img = obj.decode_stage(stage);

                compressed_size = length(compressed_data);
                actual_ratio = compressed_size / obj.original_size;

                results(stage).target_ratio = obj.stages(stage).target_ratio;
                results(stage).actual_ratio = actual_ratio;
                results(stage).compressed_size = compressed_size;
                results(stage).params = params;
                results(stage).data = compressed_data;

                if ~isempty(curr_img) && ~all(curr_img(:)==0) % Check if decoded image is not empty or all zeros
                    try
                        ssim_value = obj.calculate_ssim(obj.original, curr_img);
                        psnr_value = obj.calculate_psnr(obj.original, curr_img);
                        results(stage).ssim = ssim_value;
                        results(stage).psnr = psnr_value;
                        fprintf('PSNR: %.2f dB, SSIM: %.4f\n', psnr_value, ssim_value);
                    catch ME_metrics
                         fprintf('WARN (execute): Failed to calculate metrics for stage %d: %s\n', stage, ME_metrics.message);
                         results(stage).ssim = NaN;
                         results(stage).psnr = NaN;
                    end
                else
                    fprintf('WARN (execute): Decoded image for stage %d is empty or invalid. Metrics not calculated.\n', stage);
                    results(stage).ssim = NaN;
                    results(stage).psnr = NaN;
                end

                fprintf('Target Ratio: %.3f, Actual Ratio: %.3f, Size: %d bytes\n', ...
                        results(stage).target_ratio, results(stage).actual_ratio, results(stage).compressed_size);
            end
        end

        function decoded = decode_stage(obj, stage_num)
            % Decode image
            decoded = []; % Initialize to empty
            temp_decode_file = ''; % Initialize to prevent error if cleanup is attempted before assignment
            try
                if ~isfield(obj.stages(stage_num), 'data') || isempty(obj.stages(stage_num).data)
                    error('Data not found or compressed data is empty for stage %d', stage_num);
                end

                data_to_decode = obj.stages(stage_num).data;
                method = obj.stages(stage_num).method;

                temp_decode_file = [tempname '.imgdata']; % Generic extension
                fid = fopen(temp_decode_file, 'wb');
                if fid == -1, error('Cannot open temp file for writing decoded data.'); end
                fwrite(fid, data_to_decode);
                fclose(fid);

                current_decoded_img = [];
                if strcmp(method, "residual_compression")
                    % --- Residual Decoding Logic ---
                    try
                        metadata_len = 4; % Length of prev_data_length (uint32)
                        if length(data_to_decode) < metadata_len
                             error('Residual data too short for metadata.');
                        end
                        prev_data_len_bytes = data_to_decode(1:metadata_len);
                        prev_data_length = double(typecast(prev_data_len_bytes, 'uint32'));

                        if length(data_to_decode) < metadata_len + prev_data_length
                            error('Residual data too short for prev_data.');
                        end
                        prev_stage_data = data_to_decode(metadata_len + 1 : metadata_len + prev_data_length);
                        residual_compressed_data = data_to_decode(metadata_len + prev_data_length + 1 : end);

                        % Decode previous stage's image (which should have been decoded by standard imread)
                        % This assumes prev_stage_data is a standard image format like JPEG
                        temp_prev_img_file = [tempname '.imgdata'];
                        fid_prev = fopen(temp_prev_img_file, 'wb');
                        fwrite(fid_prev, prev_stage_data);
                        fclose(fid_prev);
                        base_image_for_residual = imread(temp_prev_img_file);
                        delete(temp_prev_img_file);

                        if isempty(base_image_for_residual)
                            error('Failed to decode base image for residual.');
                        end

                        [h_orig, w_orig, c_orig] = size(obj.original);
                        base_image_for_residual = imresize(base_image_for_residual, [h_orig, w_orig]);

                        % Decompress residual (inverse of compression step)
                        % This requires Java Deflater's inverse: InflaterInputStream
                        try
                            byte_array_input_stream = java.io.ByteArrayInputStream(uint8(residual_compressed_data));
                            inflater_input_stream = java.util.zip.InflaterInputStream(byte_array_input_stream);
                            % Read all bytes from inflater stream
                            buffer = java.io.ByteArrayOutputStream();
                            bytes_read = 0;
                            temp_buf = javaArray('int8', 1024); % int8 in Java is signed byte
                            while true
                                bytes_read = inflater_input_stream.read(temp_buf);
                                if bytes_read < 0, break; end
                                buffer.write(temp_buf, 0, bytes_read);
                            end
                            inflater_input_stream.close();
                            residual_decompressed_uint8 = buffer.toByteArray();
                            residual_values = typecast(residual_decompressed_uint8, 'int16');
                        catch ME_java_inflate
                            fprintf('WARN (decode_stage-residual): Java Inflater failed: %s. Residual might be incorrect.\n', ME_java_inflate.message);
                            % Fallback or error - for now, assume it might be uncompressed if Java fails
                            if length(residual_compressed_data) == numel(obj.original) * 2 % Heuristic for uncompressed int16
                                residual_values = typecast(uint8(residual_compressed_data), 'int16');
                            else
                                error('Cannot decompress residual without Java Inflater or known uncompressed format.');
                            end
                        end

                        if numel(residual_values) ~= numel(obj.original)
                             error('Decompressed residual size mismatch. Expected %d, got %d elements.', numel(obj.original), numel(residual_values));
                        end
                        residual_matrix = reshape(residual_values, [h_orig, w_orig, c_orig]);

                        current_decoded_img = uint8(min(max(double(base_image_for_residual) + double(residual_matrix), 0), 255));

                    catch ME_residual_decode
                        fprintf('ERROR (decode_stage): Residual decoding failed: %s\n', ME_residual_decode.message);
                        % Fallback to trying to decode the whole blob as image if residual fails badly
                        try, current_decoded_img = imread(temp_decode_file); catch; end
                    end
                else
                    % Standard decoding for JPEG, WebP (fallback), Quantized JPEG (fallback)
                    try, current_decoded_img = imread(temp_decode_file); catch; end
                end

                if exist(temp_decode_file, 'file'), delete(temp_decode_file); end

                if isempty(current_decoded_img)
                    error('imread failed to decode data for stage %d, method %s', stage_num, method);
                end

                % Ensure decoded image is resized to original dimensions if needed
                [h_orig, w_orig, ~] = size(obj.original);
                if size(current_decoded_img, 1) ~= h_orig || size(current_decoded_img, 2) ~= w_orig
                    current_decoded_img = imresize(current_decoded_img, [h_orig, w_orig]);
                end
                decoded = current_decoded_img;

            catch e
                fprintf('ERROR (decode_stage): Stage %d decoding failed: %s\n', stage_num, e.message);
                if ~isempty(temp_decode_file) && exist(temp_decode_file, 'file'), delete(temp_decode_file); end
                % Return an empty image or a black image of original size on error
                [h, w, c] = size(obj.original);
                decoded = zeros(h, w, c, 'uint8'); 
            end
        end
        
        function psnr_value = calculate_psnr(~, img1, img2)
            psnr_value = NaN; % Default
            if isempty(img1) || isempty(img2) return; end
            try
                h = min(size(img1, 1), size(img2, 1));
                w = min(size(img1, 2), size(img2, 2));
                c = min(size(img1, 3), size(img2, 3));
                if h==0 || w==0 || c==0 return; end

                img1_crop = img1(1:h, 1:w, 1:c);
                img2_crop = img2(1:h, 1:w, 1:c);
                
                psnr_value = psnr(img1_crop, img2_crop);
            catch e
                fprintf('WARN (calculate_psnr): %s\n', e.message);
            end
        end
        
        function ssim_value = calculate_ssim(~, img1, img2)
            ssim_value = NaN; % Default
            if isempty(img1) || isempty(img2) return; end
            try
                h = min(size(img1, 1), size(img2, 1));
                w = min(size(img1, 2), size(img2, 2));
                c = min(size(img1, 3), size(img2, 3));
                if h==0 || w==0 || c==0 return; end

                img1_crop = img1(1:h, 1:w, 1:c);
                img2_crop = img2(1:h, 1:w, 1:c);
                
                ssim_value = ssim(img1_crop, img2_crop);
            catch e
                fprintf('WARN (calculate_ssim): %s\n', e.message);
            end
        end
    end
end