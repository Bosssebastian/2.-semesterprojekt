import { sendStorageSlotGoto } from '$lib/server/controller';
import { json } from '@sveltejs/kit';
import type { RequestHandler } from './$types';

export const POST: RequestHandler = async ({ fetch, params }) => {
	const slotIndex = Number(params.slot);

	if (!Number.isInteger(slotIndex) || slotIndex < 0 || slotIndex > 7) {
		return json({ error: 'Invalid storage slot' }, { status: 400 });
	}

	try {
		await sendStorageSlotGoto(fetch, slotIndex);
		return new Response(null, { status: 204 });
	} catch (error) {
		return json(
			{
				error: error instanceof Error ? error.message : 'Failed to send storage slot command'
			},
			{ status: 503 }
		);
	}
};
