function dest_feats_name = vlb_describe(imdb, featsname, descriptor, varargin)
%VLB_DESCRIBE Compute descriptors for a given features
%  VLB_DESCRIBE imdb featsname descriptor
%
%  Based on descriptor type, uses either full images and a set of frames,
%  or computes descriptors using extracted patches.

% Copyright (C) 2016-2017 Karel Lenc
% All rights reserved.
%
% This file is part of the VLFeat library and is made available under
% the terms of the BSD license (see the COPYING file).

import features.*;

opts.imgExt = '.png';
opts.override = false;
opts.imids = [];
[opts, varargin] = vl_argparse(opts, varargin);

imdb = dset.factory(imdb);
if isstruct(featsname), featsname = featsname.name; end;
if iscell(featsname), featsname = fullfile(featsname); end;
impaths = {imdb.images(opts.imids).path};
imnames = {imdb.images(opts.imids).name};
descriptor = features.factory('desc', descriptor, varargin{:});

dets_dir = vlb_path('features', imdb, struct('name', featsname));
assert(isdir(dets_dir), 'Cannot find frames for %s - %s', ...
  imdb.name, featsname);

dest_feats_name = fullfile(featsname, descriptor.name);
desc_dest_dir = vlb_path('features', imdb, dest_feats_name);
vl_xmkdir(desc_dest_dir);

fprintf('Computing descriptor `%s` for %d images of features `%s` from a dset `%s`.\n', ...
  descriptor.name, numel(impaths), featsname, imdb.name);
fprintf('Resulting features are going to be stored in:\n%s.\n', desc_dest_dir);
status = utls.textprogressbar(numel(impaths), 'startmsg', ...
  sprintf('Computing %s ', descriptor.name), 'updatestep', 1);
for si = 1:numel(impaths)
  imname = imnames{si};
  feats_path = fullfile(desc_dest_dir, imname);
  feats = utls.features_load(feats_path, 'checkOnly', true);
  if ~isempty(feats) && ~opts.override, status(si); continue; end
  
  dets_path = fullfile(dets_dir, imname);
  det_feats = utls.features_load(dets_path);
  if isempty(det_feats) || ~isfield(det_feats, 'frames')
    error('Frames in %s not found.', dets_path);
  end
  
  switch descriptor.describes
    case 'patches'
      patches_path = fullfile(vlb_path('patches', imdb, ...
        struct('name', featsname)), [imname, '.png']);
      if ~exist(patches_path, 'file')
        error('Patches %s not found.', patches_path);
      end
      [patches, scalingFactor] = utls.patches_load(patches_path);
      desc_feats = descriptor.fun(patches);
      desc_feats.scalingFactor = scalingFactor;
    case 'frames'
      im = imread(impaths{si});
      desc_feats = descriptor.fun(im, det_feats);
  end
  feats = vl_override(det_feats, desc_feats);
  utls.features_save(feats_path, feats);
  status(si);
end

end

